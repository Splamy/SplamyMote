// See https://aka.ms/new-console-template for more information
if (false) {
    var cycles = new List<int>();
    for(int i = 0; i < 50; i++) {
        cycles.Add((500 * Random.Shared.Next(1, 5))+ Random.Shared.Next(0, 50) - 25);
    }
    Console.WriteLine($"new int[] {{ {(string.Join(", ", cycles))} }}");
}

var demo = new int[] { 479, 1985, 503, 494, 1520, 504, 523, 1503, 993, 2009, 2022, 1979, 987, 1524, 990, 1496, 1486, 1493, 2011, 480, 1515, 2010, 1505, 485, 1979, 1483, 1513, 1504, 1023, 496, 990, 2016, 475, 520, 1981, 1522, 522, 514, 2019, 1491, 981, 1009, 2010, 1004, 1975, 1988, 987, 1513, 1478, 1000 };

var tmp = new List<int>();

for(int i = 0; i < demo.Length; i++)
for(int j = i; j < demo.Length; j++)
{
    var a = demo[i];
    var b = demo[j];
    tmp.Add((int)Math.Round(Math.Max(a, b) / (float)Math.Min(a, b)));
}

Console.WriteLine(string.Join(", ", tmp));