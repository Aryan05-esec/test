using System;
using System.Management;
using System.Windows.Forms;

namespace BootableTool
{
    public partial class Form1 : Form
    {
        private ListBox usbList;
        private Button refreshButton;

        public Form1()
        {
            InitializeComponent();

            // Window
            this.Text = "eSec Forte Bootable Tool";
            this.Width = 700;
            this.Height = 500;
            this.StartPosition = FormStartPosition.CenterScreen;

            // Title
            Label title = new Label();
            title.Text = "USB Device Detection";
            title.Left = 30;
            title.Top = 25;
            title.Width = 500;
            title.Font = new System.Drawing.Font("Arial", 18);

            // USB list
            usbList = new ListBox();
            usbList.Left = 30;
            usbList.Top = 80;
            usbList.Width = 620;
            usbList.Height = 280;

            // Refresh button
            refreshButton = new Button();
            refreshButton.Text = "Refresh USB";
            refreshButton.Left = 30;
            refreshButton.Top = 390;
            refreshButton.Width = 150;
            refreshButton.Height = 40;

            refreshButton.Click += RefreshButton_Click;

            // Add controls
            this.Controls.Add(title);
            this.Controls.Add(usbList);
            this.Controls.Add(refreshButton);

            // Load USB devices
            LoadUSBDevices();
        }

        private void RefreshButton_Click(object? sender, EventArgs e)
        {
            LoadUSBDevices();
        }

        private void LoadUSBDevices()
        {
            usbList.Items.Clear();

            try
            {
                using ManagementObjectSearcher searcher =
                    new ManagementObjectSearcher(
                        "SELECT * FROM Win32_DiskDrive WHERE InterfaceType='USB'");

                foreach (ManagementObject disk in searcher.Get())
                {
                    string model = disk["Model"]?.ToString() ?? "Unknown";
                    string size = disk["Size"]?.ToString() ?? "0";
                    string deviceId = disk["DeviceID"]?.ToString() ?? "Unknown";

                    long sizeBytes = 0;

                    long.TryParse(size, out sizeBytes);

                    double sizeGB =
                        sizeBytes / 1024.0 / 1024.0 / 1024.0;

                    string information =
                        $"USB Disk | {model} | {sizeGB:F1} GB | {deviceId}";

                    usbList.Items.Add(information);
                }

                if (usbList.Items.Count == 0)
                {
                    usbList.Items.Add("No USB disk detected.");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    "Error detecting USB:\n\n" + ex.Message,
                    "Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
            }
        }
    }
}