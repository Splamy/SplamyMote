using System.Net;
using System.Net.Sockets;

namespace SplamyMoteServer.Data;

public class IRCommunicator : BackgroundService
{
    private readonly UdpClient udp = new(58915);

    public event Action<IRData>? IRReceived;

    public IRCommunicator()
    {
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        await ReceiveMessageLoop();
    }

    private async Task ReceiveMessageLoop()
    {
        while (true)
        {
            var udpReceive = await udp.ReceiveAsync();
            var data = udpReceive.Buffer;

            var irm = IRMessage.Decode(data);
            if (irm == null)
                continue;

            if (irm.PkgType != Pkg.IR_Data)
                continue;

            var irData = irm.GetIRData();
            IRReceived?.Invoke(irData);
        }
    }

    public async Task Send(IRMessage message)
    {
        var send = message.Encode();
        await udp.SendAsync(send, send.Length, new IPEndPoint(new IPAddress(new byte[] { 192, 168, 178, 169 }), 58914));
    }
}
