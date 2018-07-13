using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
namespace raytrace_display
{
    public partial class Form1 : Form
    {
        //private Graphics g;
        private Point mousePos;
        
        public Form1()
        {
            InitializeComponent();
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            
            //g = pictureBox1.CreateGraphics();

        }

        private void button1_Click(object sender, EventArgs e)
        {
            pictureBox1.BackgroundImage = Image.FromFile(("c:\\MinGW\\raytrace_out_"+textBox1.Text+".png"));
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            int index = (e.Y * 1024 + e.X);
            button1.Text = String.Format("{0} {1} {2} ({3} {4} {5})", e.X, e.Y, index, loc[3*index], loc[3*index+1], loc[3*index+2]);
            
            //if (count[index] > 0) MessageBox.Show("hi");
            button2.Text = String.Format("{0} {1} {2} {3} {4}", arr[3*index], arr[3*index + 1], arr[3*index + 2], rad2[index], count[index]);
            mousePos.X = e.X;
            mousePos.Y = e.Y;
            pictureBox1.Refresh();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //g = pictureBox1.CreateGraphics();
            arr = new double[1024 * 1024 * 3];
            rad2 = new double[1024 * 1024];
            count = new int[1024 * 1024];
            loc = new double[1024 * 1024 * 3];
        }

        private void pictureBox1_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            //g.Clear(Color.White);
            g.DrawRectangle(Pens.Green, new Rectangle(mousePos.X, mousePos.Y, 6, 6));
            //g.Save();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            FileStream fs = new FileStream("c:\\MinGW\\raytrace_photon_" + textBox1.Text + ".dat", FileMode.Open);

            byte[] buf = new byte[128];
            for (int i = 0; i < 1024 * 1024 * 3; i++)
            {

                fs.Read(buf, 0, sizeof(double));
                arr[i] = BitConverter.ToDouble(buf, 0);
            }
            for (int i = 0; i < 1024 * 1024; i++)
            {
                fs.Read(buf, 0, sizeof(double));
                rad2[i] = BitConverter.ToDouble(buf, 0);
                fs.Read(buf, 0, sizeof(int));

                //if (buf[0] > 0) MessageBox.Show(String.Format("{7} {5} {6} {0} {1} {2} {3} {4}", buf[0], buf[1] ,buf[2], buf[3], BitConverter.ToInt32(buf, 0), i/1024, i%1024, i));
                count[i] = BitConverter.ToInt32(buf, 0);
                fs.Read(buf, 0, sizeof(double));
                loc[3*i] = BitConverter.ToDouble(buf, 0);
                fs.Read(buf, 0, sizeof(double));
                loc[3 * i+1] = BitConverter.ToDouble(buf, 0);
                fs.Read(buf, 0, sizeof(double));
                loc[3 * i+2] = BitConverter.ToDouble(buf, 0);
            }
            fs.Close();
        }
        double[] arr;
        double[] rad2;
        int[] count;
        double[] loc;
        private void openFileDialog2_FileOk(object sender, CancelEventArgs e)
        {

        }
    }
}
