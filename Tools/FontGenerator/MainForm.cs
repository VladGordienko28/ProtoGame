//-----------------------------------------------------------------------------
//	MainForm.cs: Font Generation main form
//	Created by Vlad Gordienko, Sep 2018
//-----------------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Drawing.Text;

namespace FontGenerator
{
    public partial class MainForm : Form
    {
        const string DEFAULT_CHAR_SET = "!\"#$%&'()*+,-./0123456789:;<=> ?@" +
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" +
            "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя";

        const string IMAGE_POSTFIX = "_img";

        const string RESOURCE_PREFIX = "Fonts.";

        const int MINIMAL_ATLAS_SIZE = 256;

        int roundToNextPowOfTwo( int value )
        {
            return (int)Math.Pow(2, Math.Ceiling(Math.Log(value) / Math.Log(2)));
        }

        public MainForm()
        {
            InitializeComponent();
        }

        private void chooseButton_Click(object sender, EventArgs e)
        {
            if( fontDialog.ShowDialog() != DialogResult.Cancel )
            {
                Font font = fontDialog.Font;
                textBox1.Font = font;
                buildButton.Enabled = true;
                fontInfoLabel.Text = "Font Name: " + font.Name + "\n";
                fontInfoLabel.Text += "Font Size: " + font.SizeInPoints.ToString();
            }
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            textBox1.Text = DEFAULT_CHAR_SET;
        }

        private void buildButton_Click(object sender, EventArgs e)
        {
            var font = fontDialog.Font;
            int padding = (int)paddingSpinner.Value;

            string name = string.Format("{0}_{1}", font.Name.Replace(" ", ""), (int)font.Size);
            string fileName = name + ".ffnt";
            string imageName = name + IMAGE_POSTFIX + ".png";
            StreamWriter file = new StreamWriter( fileName );

            Bitmap tempImage = new Bitmap( MINIMAL_ATLAS_SIZE, MINIMAL_ATLAS_SIZE );
            Graphics tempGraphics = Graphics.FromImage( tempImage );
            tempGraphics.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;

            file.WriteLine( "{" );
            {
                file.WriteLine("  Name: \"{0}\",", font.Name);
                file.WriteLine("  Image: \"{0}\",", RESOURCE_PREFIX + name + IMAGE_POSTFIX);

                file.WriteLine("  Glyphs: ");
                file.WriteLine("  [");
                {
                    string glyphs = textBox1.Text;
                    
                    var stringFormat = new StringFormat();
                    stringFormat.Alignment = StringAlignment.Near;
                    stringFormat.LineAlignment = StringAlignment.Near;
                    stringFormat.Trimming = StringTrimming.None;
                    stringFormat.FormatFlags = StringFormatFlags.FitBlackBox | StringFormatFlags.NoClip | StringFormatFlags.LineLimit;
                    //var stringFormat = StringFormat.GenericTypographic;

                    TextFormatFlags textFlags = TextFormatFlags.NoPadding | TextFormatFlags.NoClipping | TextFormatFlags.NoPrefix;

                    // find the biggest image size
                    int maxXSize = 0;
                    int maxYSize = 0;

                    for (int i = 0; i < glyphs.Length; i++)
                    {
                        char thisChar = glyphs[i];
                        //SizeF size = tempGraphics.MeasureString(thisChar.ToString(), font, MINIMAL_ATLAS_SIZE, stringFormat);
                        SizeF size = TextRenderer.MeasureText( tempGraphics, thisChar.ToString(), font, new Size(MINIMAL_ATLAS_SIZE, MINIMAL_ATLAS_SIZE), textFlags);

                        maxXSize = size.Width > maxXSize ? (int)size.Width : maxXSize;
                        maxYSize = size.Height > maxYSize ? (int)size.Height : maxYSize;
                    }

                    // add padding to maximum glyph size
                    maxXSize += padding;
                    maxYSize += padding;

                    // compute final atlas size
                    int charsPerRow = (int)Math.Ceiling( Math.Sqrt(glyphs.Length) );
                    int atlasXSize = roundToNextPowOfTwo(charsPerRow * maxXSize);
                    int realCharsPerRow = (int)Math.Floor((float)atlasXSize / (float)maxXSize);
                    int charsPerColumn = (int)Math.Ceiling((float)glyphs.Length / (float)realCharsPerRow);
                    int atlasYSize = roundToNextPowOfTwo(charsPerColumn * maxYSize);

                    // render glyphs to final image
                    Bitmap finalImage = new Bitmap(atlasXSize, atlasYSize);
                    Graphics finalGraphics = Graphics.FromImage(finalImage);
                    finalGraphics.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;

                    int walkX = padding, walkY = padding;

                    for (int i = 0; i < glyphs.Length; i++)
                    {
                        char thisChar = glyphs[i];
                        //SizeF size = finalGraphics.MeasureString(thisChar.ToString(), font, MINIMAL_ATLAS_SIZE, stringFormat);
                        SizeF size = TextRenderer.MeasureText(finalGraphics, thisChar.ToString(), font, new Size(MINIMAL_ATLAS_SIZE, MINIMAL_ATLAS_SIZE), textFlags);

                        int sizeX = (int)size.Width;
                        int sizeY = (int)size.Height;

                        // check line overflow
                        if( walkX + sizeX > atlasXSize)
                        {
                            walkX = padding;
                            walkY += maxYSize;
                        }

                        //finalGraphics.DrawString(thisChar.ToString(), font, Brushes.White, walkX, walkY, stringFormat);
                        Color foreColor = Color.FromArgb( 255, 255, 255, 255 );

                        TextRenderer.DrawText( finalGraphics, thisChar.ToString(), font, new Point( walkX, walkY ), foreColor, textFlags);

                        //DrawText(System.Drawing.IDeviceContext dc, string text, System.Drawing.Font font, System.Drawing.Point pt, System.Drawing.Color foreColor);

                        file.WriteLine( "    {{ C: {0}, X: {1}, Y: {2}, W: {3}, H: {4} }}{5}",
                            (int)thisChar, walkX, walkY, sizeX, sizeY, i < ( glyphs.Length - 1 ) ? "," : "" );

                        walkX += maxXSize;
                    }

                    // filter image
                    for( int y = 0; y < finalImage.Height; ++y )
                    {
                        for( int x = 0; x < finalImage.Width; ++x )
                        {
                            Color c = finalImage.GetPixel( x, y );
                            finalImage.SetPixel( x, y, Color.FromArgb((c.R + c.G + c.B) / 3, 255, 255, 255));
                        }
                    }

                    finalImage.Save( imageName, System.Drawing.Imaging.ImageFormat.Png );
                }
                file.WriteLine("  ]");
            }
            file.WriteLine( "}" );
            file.Close();

            MessageBox.Show("Font generated successfully", "Font Generator", MessageBoxButtons.OK);
        }
    }
}
