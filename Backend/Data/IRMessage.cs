using System.Buffers.Binary;

namespace SplamyMoteServer.Data;

public class IRMessage
{
    public Pkg PkgType { get; set; }
    public PackMode PackMode { get; set; }
    public byte[] Data { get; set; }

    public static IRMessage Decode(ReadOnlySpan<byte> rawData)
    {
        IRMessage message = new();
        message.PkgType = (Pkg)rawData[0];
        if (!Enum.IsDefined(message.PkgType)) throw new InvalidDataException("Invalid packet type");
        message.PackMode = (PackMode)rawData[1];
        if (!Enum.IsDefined(message.PackMode)) throw new InvalidDataException("Invalid packmode type");
        var length = BinaryPrimitives.ReadUInt16LittleEndian(rawData[2..]);
        message.Data = rawData.Slice(4, length).ToArray();
        return message;
    }

    public byte[] Encode()
    {
        var rawData = new byte[4 + Data.Length];
        rawData[0] = (byte)PkgType;
        rawData[1] = (byte)PackMode;
        BinaryPrimitives.WriteUInt16LittleEndian(rawData.AsSpan(2), (ushort)Data.Length);
        Data.AsSpan().CopyTo(rawData.AsSpan(4));
        return rawData;
    }

    public IRData GetIRData()
    {
        if (PkgType != Pkg.IR_Data)
            throw new InvalidOperationException();

        if (PackMode == PackMode.RawU16)
            return IRData.DecodeRawU16(Data);

        throw new NotImplementedException();
    }

    public void SetIRData(IRData iRData)
    {
        PkgType = Pkg.IR_Data;
        PackMode = PackMode.RawU16;
        Data = iRData.EncodeRawU16();
    }

    public void SetIpInit() => SetNoDataPkg(Pkg.IpInit);
    public void SetIRLearn() => SetNoDataPkg(Pkg.Set_IRLearn);
    public void SetIRSend() => SetNoDataPkg(Pkg.Set_IRSend);


    public void SetNoDataPkg(Pkg type)
    {
        PkgType = type;
        PackMode = 0;
        Data = Array.Empty<byte>();
    }
}

public enum PackMode : byte
{
    Invalid = 0,
    BitMask = 1,
    RawU16 = 2,
}

public enum Pkg : byte
{
    Invalid = 0,
    IpInit = 1 /* Host -> MCU */,
    IpOk = 2 /* Host <- MCU */,
    Set_IRLearn = 3 /* Host -> MCU */,
    Set_IRSend = 4 /* Host -> MCU */,
    IR_Data = 5 /* Host <-> MCU */,
}