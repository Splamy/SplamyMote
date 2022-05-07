using System.Text.Encodings.Web;
using System.Text.Json;

namespace SplamyMoteServer.Data;

public class IRManager
{
    public List<MoRemote> Remotes { get; set; } = new();

    public IRManager()
    {
        Load();
    }

    public void Save()
    {
        var data = JsonSerializer.SerializeToUtf8Bytes(Remotes, new JsonSerializerOptions()
        {
            WriteIndented = true,
            Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping
        });
        File.WriteAllBytes("mote.json", data);
    }

    public void Load()
    {
        if (!File.Exists("mote.json"))
            return;
        var data = File.ReadAllBytes("mote.json");
        Remotes = JsonSerializer.Deserialize<List<MoRemote>>(data)!;
    }
}

public class MoRemote
{
    public List<MoButton> Buttons { get; set; } = new();
    public string Name { get; set; } = "";
}

public class MoButton
{
    public IRData? Data { get; set; }

    // UI
    public int PosX { get; set; }
    public int PosY { get; set; }
    public string Label { get; set; } = "";
}