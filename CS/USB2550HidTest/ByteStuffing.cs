using System;
using System.Collections.Generic;
using System.Linq;

namespace bproj
{
    public class ByteStuffing
    {
        public enum ReserverChars : byte
        {
            SOF = (byte)'&',
            EOF = (byte)'%',
            ESC = (byte)'\\',
            NOP = (byte)'*',
        }

        bool startFound = false;
        bool esc = false;
        List<byte> dataBuffer = new List<byte>();

        public void Unstuff(List<byte> data)
        {
            int len = data.Count();
            for (int i = 0; i < len; i++)
            {
                bool record = false;

                if (esc)
                {
                    record = true;
                    esc = false;
                }
                else
                {
                    switch (data[i])
                    {
                        case (byte)ReserverChars.ESC:
                            esc = true;
                            break;
                        case (byte)ReserverChars.SOF:
                            startFound = true;
                            dataBuffer.Clear();
                            break;
                        case (byte)ReserverChars.EOF:
                            startFound = false;
                            OnFrameCollected(dataBuffer);
                            break;
                        case (byte)ReserverChars.NOP:
                            break;
                        default:
                            record = true;
                            break;
                    }
                }

                if (record && startFound)
                    dataBuffer.Add(data[i]);
            } 
        }

        public List<byte> Stuff(List<byte> frame)
        {
            List<byte> dataOut = new List<byte>();
            dataOut.Add((byte) ReserverChars.SOF);
            foreach(byte b in frame)
            {
                if(Enum.IsDefined(typeof(ReserverChars), b))
                    dataOut.Add((byte)ReserverChars.ESC);
                dataOut.Add(b);
            }

            dataOut.Add((byte)ReserverChars.EOF);
            return dataOut;           
        }


        public event Action<List<byte>> OnFrameCollected;
    }


}
