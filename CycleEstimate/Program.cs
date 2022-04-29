// See https://aka.ms/new-console-template for more information
using System.Globalization;
using System.Text.Json;

// var cycles = new List<int>();
// for (int i = 0; i < 50; i++)
// {
// 	cycles.Add((500 * Random.Shared.Next(1, 5)) + Random.Shared.Next(0, 50) - 25);
// }
// Console.WriteLine($"new int[] {{ {(string.Join(", ", cycles))} }}");

//var demo = new List<int> { 2392, 602,  1196, 602,  598, 600,  1194, 600,  600, 600,  1196, 602,  598, 600,  598, 600,  596, 600,  1196, 600,  598, 600,  1196, 600,  1196, 602,  598, 600,  1194, 602,  596, 600,  598, 600,  596, 600,  1194, 602,  1196, 600,  1194, 12498,  2392, 602,  1196, 600,  598, 598,  1196, 600,  598, 600,  1194, 602,  598, 600,  598, 598,  598, 600,  1196, 602,  596, 600,  1196, 602,  1196, 600,  596, 600,  1196, 600,  598, 600,  598, 600,  598, 600,  1196, 600,  1194, 600,  1196, 12500,  2392, 602,  1196, 600,  598, 600,  1194, 602,  598, 600,  1196, 602,  598, 600,  598 };

// MNET (Small)
//var demo = new List<int> { 864, 910,  864, 910,  1752, 1800,  1752, 1800,  1752, 910,  864, 910,  864, 910,  864, 910,  864, 1800,  1752 };

// XORO
//var demo = new List<int> { 8996, 4544,  534, 1714,  532, 576,  534, 574,  534, 576,  534, 576,  534, 576,  534, 578,  532, 578,  534, 574,  536, 1712,  532, 1714,  532, 1716,  532, 1712,  534, 1712,  534, 1712,  534, 1714,  532, 1712,  534, 576,  534, 576,  534, 1714,  532, 576,  534, 576,  534, 1710,  536, 576,  534, 574,  536, 1712,  534, 1712,  534, 576,  534, 1712,  534, 1712,  534, 576,  534, 1712,  534 };

// Hisense
//var demo = new List<int> { 8906, 4304,  554, 554,  556, 556,  554, 554,  554, 554,  554, 556,  554, 556,  552, 556,  554, 556,  554, 1666,  552, 1666,  554, 1664,  554, 1666,  554, 1664,  554, 1664,  554, 556,  554, 1664,  554, 556,  552, 1666,  554, 556,  554, 554,  554, 556,  554, 556,  554, 554,  554, 554,  556, 1664,  554, 554,  554, 1666,  554, 1664,  554, 1666,  554, 1666,  554, 1666,  554, 1664,  556 };

// Mömax
var demo = new List<int> {  8836, 4434,  440, 542,  470, 1492,  470, 512,  470, 510,  466, 514,  466, 1496,  468, 514,  468, 512,  468, 1496,  468, 512,  468, 1494,  470, 1494,  468, 1496,  468, 512,  470, 1494,  470, 1492,  470, 512,  486, 494,  466, 516,  468, 1496,  468, 1492,  470, 512,  468, 514,  474, 508,  466, 1496,  474, 1490,  468, 1494,  444, 538,  472, 510,  470, 1492,  468, 1494,  472, 1490,  472 };

// var tmp = new List<int>();

// for(int i = 0; i < demo.Length; i++)
// for(int j = i; j < demo.Length; j++)
// {
//     var a = demo[i];
//     var b = demo[j];
//     tmp.Add((int)Math.Round(Math.Max(a, b) / (float)Math.Min(a, b)));
// }

var prev = -1;
var bucket = new List<int>();
var preBuckets = new List<List<int>>();
const float pctl = 0.9f;

// Step 1: Bucketize 90%ile to find buckets

foreach (var i in demo
	.OrderBy(x => x)
	.Take((int)(demo.Count * (1 - pctl))..(int)(demo.Count * pctl)))
{
	if (i > prev + (prev / 3) && bucket.Count > 0)
	{
		preBuckets.Add(bucket);
		bucket = new List<int>();
	}
	bucket.Add(i);
	prev = i;
}
preBuckets.Add(bucket);

Console.WriteLine("raw buckets: {0}", JsonSerializer.Serialize(preBuckets));

// Step 2: Use average of buckets to group the correct buckets

var preBucketsAvgs = preBuckets.OrderByDescending(b => b.Count)
		.Take(3)
		.Select(b => b.Average());
var tolerance = (int)(preBucketsAvgs.Min() / 3);
var buckets = demo.GroupBy(x => (int)Math.Round(x / (float)tolerance) * tolerance).Select(x => x.ToList()).ToList();

Console.WriteLine("final buckets: {0}", JsonSerializer.Serialize(buckets));

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

var cleaned = demo.Select(i => (int)Math.Round(i / (double)cBasei10))
				  .ToArray();

Console.WriteLine("Val:{0}", JsonSerializer.Serialize(cleaned));
