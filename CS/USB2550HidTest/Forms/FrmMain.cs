using System;
using System.Collections.Generic;
using System.Windows.Forms;
using UsbHid;
using UsbHid.USB.Classes.Messaging;
using bproj;
using System.Linq;

namespace USB2550HidTest.Forms
{
    public partial class FrmMain : Form
    {
        BASUSB bub = new BASUSB();
        BVProtocol prot = new BVProtocol();

        public FrmMain()
        {
            InitializeComponent();
        }

        private void FrmMainLoad(object sender, EventArgs e)
        {
            bub.OnPackageRecieved += Bub_OnPackageRecieved;
            prot.OnRawDataOut = prot_RawOut;
        }

        private void Bub_OnPackageRecieved(List<byte> data)
        {
            prot.RawDataIn(data);
        }

        private void prot_RawOut(List<byte> data)
        {
            bub.SendPackage(data);
        }

        private void Button1Click(object sender, EventArgs e)
        {
            prot.SendRequest(0x00, null, ResponseReceived);

        }

        private void ResponseReceived(Command cmd)
        {
            int size = 0;

            for(int i=0; i<4; i++)
            {
                size += cmd.Data[i] << (i * 8);
            }

        }


    }


    public class BASUSB
    {
        public event Action<List<byte>> OnPackageRecieved;

        public UsbHidDevice Device;

        public BASUSB()
        {
            Device = new UsbHidDevice(0x0483, 0x5750);

            Device.DataReceived += DeviceDataReceived;
            Device.Connect();
        }

        private void DeviceDataReceived(byte[] data)
        {
            List<byte> ldata = new List<byte>();
            ldata.AddRange(data);
            OnPackageRecieved.Invoke(ldata);
        }

        public void SendPackage(List<byte> data)
        {
            if (!Device.IsDeviceConnected) return;

            int i = 0;

            while(i<data.Count)
            {
                byte[] buf = Enumerable.Repeat((byte)ByteStuffing.ReserverChars.NOP, 64).ToArray();
                int len = data.Count - i;

                Array.Copy(data.ToArray(), 0, buf, 0, len);

                var command = new CommandMessage(buf);
                Device.SendMessage(command);
                i += len;
            }
        }
    }

}
