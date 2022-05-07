using System.Globalization;
using System.Runtime.InteropServices;

namespace SplamyMoteServer.Data;

public record IRData(ushort Base, byte[] Times, bool Repeating = false)
{
    public byte[] EncodeRawU16()
    {
        var u16 = Times.Select(x => (ushort)(Base * x)).ToArray();
        var data = MemoryMarshal.Cast<ushort, byte>(u16).ToArray();
        return data;
    }

    public static IRData DecodeRawU16(ReadOnlySpan<byte> rawData)
    {
        var rawMeasure = MemoryMarshal.Cast<byte, ushort>(rawData);
        return IRData
            .Clean(rawMeasure.ToArray().Skip(1).Select(x => 2 * x).ToList())
            .Depatternize();
    }

    public static IRData Clean(IReadOnlyList<int> rawData)
    {
        int prev = 0;
        var bucket = new List<int>();
        var preBuckets = new List<List<int>>();
        const float pctl = 0.9f;

        // Step 1: Bucketize 90%ile to find buckets

        foreach (var i in rawData
            .OrderBy(x => x)
            .Take((int)(rawData.Count * (1 - pctl))..(int)(rawData.Count * pctl)))
        {
            if (i > prev + (prev / 3) && bucket.Count > 0)
            {
                preBuckets.Add(bucket);
                bucket = new();
            }
            bucket.Add(i);
            prev = i;
        }
        preBuckets.Add(bucket);

        //Console.WriteLine("raw buckets: {0}", JsonSerializer.Serialize(preBuckets));

        // Step 2: Use average of buckets to group the correct buckets

        var preBucketsAvgs = preBuckets.OrderByDescending(b => b.Count)
                .Take(3)
                .Select(b => b.Average());
        var tolerance = (int)(preBucketsAvgs.Min() / 3);
        var buckets = rawData.GroupBy(x => (int)Math.Round(x / (float)tolerance) * tolerance).Select(x => x.ToList()).ToList();

        //Console.WriteLine("final buckets: {0}", JsonSerializer.Serialize(buckets));

        // Step 3: Get the base bucket

        var (cBucket, cBase) = buckets.OrderByDescending(b => b.Count)
                           .Take(3)
                           .Select(b => (bucket: b, avg: b.Average()))
                           .MinBy(b => b.avg);
        var cVariance = Math.Sqrt(cBucket.Sum(x => Math.Pow(x - cBase, 2)) / (cBucket.Count - 1));

        var cBasei = (int)Math.Round(cBase);
        var cBasei10 = (int)Math.Round(cBase / 10) * 10;
        Console.WriteLine("Raw Base {0}±{1}  Rounded Base {2}  Rounded10 Base {3}",
            cBase.ToString(CultureInfo.InvariantCulture),
            Math.Round(cVariance, 2).ToString(CultureInfo.InvariantCulture),
            cBasei,
            cBasei10);

        // Step 4: Clean raw data with the calculated base

        var cleaned = rawData.Select(i => (byte)Math.Round(i / (double)cBasei10))
                          .ToArray();

        Console.WriteLine("Val:{0}", string.Join(", ", cleaned));

        return new IRData((ushort)cBasei10, cleaned);
    }

    private IRData Depatternize()
    {
        if (Times.Length <= 2)
            return this;

        var max = Times.Max();
        var avg = (byte)Times.Select(x => (int)x).Average();
        if (max <= 2 * avg)
            return this;

        var patternStart = Array.FindIndex(Times, i => i == max);
        var patternEnd = Array.FindIndex(Times, patternStart + 1, i => i == max);

        if (patternStart == -1 || patternEnd == -1 || patternStart == patternEnd)
            return this;

        var pattern = Times[patternStart..patternEnd];
        var repeating = Times.Count(x => x == max) > 1;


        return this with { Times = pattern, Repeating = repeating };
    }
}
