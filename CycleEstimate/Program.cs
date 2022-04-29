// See https://aka.ms/new-console-template for more information
if (false) {
    var cycles = new List<int>();
    for(int i = 0; i < 50; i++) {
        cycles.Add((500 * Random.Shared.Next(1, 5))+ Random.Shared.Next(0, 50) - 25);
    }
    Console.WriteLine($"new int[] {{ {(string.Join(", ", cycles))} }}");
}

var demo = new int[] { 1,4403,2225,237,254,250,731,250,241,249,241,251,240,237,744,243,249,249,241,250,731,237,253,237,744,250,732,249,732,236,254,238,744,237,744,249,241,237,254,237,743,237,744,237,254,250,240,236,254,237,253,236,745,237,745,236,254,237,254,237,744,236,746,236,745,237,744,238 };

var tmp = new List<int>();

for(int i = 0; i < demo.Length; i++)
for(int j = i; j < demo.Length; j++)
{
    var a = demo[i];
    var b = demo[j];
    tmp.Add((int)Math.Round(Math.Max(a, b) / (float)Math.Min(a, b)));
}

Console.WriteLine(string.Join(", ", tmp));
