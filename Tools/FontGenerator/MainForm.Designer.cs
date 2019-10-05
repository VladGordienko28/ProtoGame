﻿//-----------------------------------------------------------------------------
//	MainForm.Designer.cs: Font Generation main form designer
//	Created by Vlad Gordienko, Sep 2018
//-----------------------------------------------------------------------------
namespace FontGenerator
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.fontDialog = new System.Windows.Forms.FontDialog();
            this.chooseButton = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.buildButton = new System.Windows.Forms.Button();
            this.fontInfoLabel = new System.Windows.Forms.Label();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // fontDialog
            // 
            this.fontDialog.ShowEffects = false;
            // 
            // chooseButton
            // 
            this.chooseButton.Location = new System.Drawing.Point(3, 3);
            this.chooseButton.Name = "chooseButton";
            this.chooseButton.Size = new System.Drawing.Size(101, 34);
            this.chooseButton.TabIndex = 0;
            this.chooseButton.Text = "Choose Font";
            this.chooseButton.UseVisualStyleBackColor = true;
            this.chooseButton.Click += new System.EventHandler(this.chooseButton_Click);
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.AutoSize = true;
            this.panel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.fontInfoLabel);
            this.panel1.Controls.Add(this.buildButton);
            this.panel1.Controls.Add(this.chooseButton);
            this.panel1.Location = new System.Drawing.Point(12, 12);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(221, 68);
            this.panel1.TabIndex = 1;
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 87);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(460, 362);
            this.textBox1.TabIndex = 2;
            // 
            // buildButton
            // 
            this.buildButton.Enabled = false;
            this.buildButton.Location = new System.Drawing.Point(110, 3);
            this.buildButton.Name = "buildButton";
            this.buildButton.Size = new System.Drawing.Size(106, 34);
            this.buildButton.TabIndex = 1;
            this.buildButton.Text = "Build";
            this.buildButton.UseVisualStyleBackColor = true;
            this.buildButton.Click += new System.EventHandler(this.buildButton_Click);
            // 
            // fontInfoLabel
            // 
            this.fontInfoLabel.AutoSize = true;
            this.fontInfoLabel.Location = new System.Drawing.Point(3, 40);
            this.fontInfoLabel.Name = "fontInfoLabel";
            this.fontInfoLabel.Size = new System.Drawing.Size(65, 26);
            this.fontInfoLabel.TabIndex = 2;
            this.fontInfoLabel.Text = "Font Name: \r\nFont Size: ";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 461);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "[Fluorine Engine] Font Generator";
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.FontDialog fontDialog;
        private System.Windows.Forms.Button chooseButton;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button buildButton;
        private System.Windows.Forms.Label fontInfoLabel;
    }
}
