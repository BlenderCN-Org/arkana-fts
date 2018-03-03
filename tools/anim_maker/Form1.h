#pragma once

#define PACK_NAME "fts_packer.exe"

namespace anim_maker {
    using namespace System;
    using namespace System::IO;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    /// <summary>
    /// Summary for Form1
    ///
    /// WARNING: If you change the name of this class, you will need to change the
    ///          'Resource File Name' property for the managed resource compiler tool
    ///          associated with all .resx files this class depends on.  Otherwise,
    ///          the designers will not be able to interact properly with localized
    ///          resources associated with this form.
    /// </summary>
    public ref class Form1:public System::Windows::Forms::Form {
      public:
        Form1(void) {
            InitializeComponent();
            this->DirNameIn->Text = Environment::CurrentDirectory;
            this->folderBrowserDialog1->SelectedPath =
                Environment::CurrentDirectory;
            this->FileNameOut->Text =
                Environment::CurrentDirectory + "\\my_animation.anim";
            this->saveFileDialog1->FileName =
                Environment::CurrentDirectory + "\\my_animation.anim";
            this->saveFileDialog1->DefaultExt = ".anim";
            this->folderBrowserDialogExe->SelectedPath =
                Environment::CurrentDirectory;
            this->ExeName->Text = Environment::CurrentDirectory;

            if(!File::Exists(this->ExeName->Text + "\\" PACK_NAME)) {
                this->ExeName->ForeColor = System::Drawing::Color::Red;
            } else {
                this->ExeName->ForeColor = System::Drawing::Color::Black;
            } this->comboFileType->SelectedIndex = 0;

            iCurrFrame = 0;
        }

      protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~Form1() {
            if(components) {
                delete components;
            }
        }

      private:System::Windows::Forms::
            SaveFileDialog ^ saveFileDialog1;
      private:System::Windows::Forms::Button ^ BrowseButtonOut;
      private:System::Windows::Forms::TextBox ^ FileNameOut;
      private:System::Windows::Forms::Button ^ BrowseButtonIn;
      private:System::Windows::Forms::TextBox ^ DirNameIn;
      private:System::Windows::Forms::
            FolderBrowserDialog ^ folderBrowserDialog1;
      private:System::Windows::Forms::Label ^ label1;
      private:System::Windows::Forms::Label ^ label2;
      private:System::Windows::Forms::ListBox ^ BrowseListBox;
      private:System::Windows::Forms::PictureBox ^ pictureBox1;
      private:System::Windows::Forms::ListBox ^ OkListBox;
      private:System::Windows::Forms::Button ^ LeftToRight;
      private:System::Windows::Forms::Button ^ RightToLeft;
      private:System::Windows::Forms::Label ^ label3;
      private:System::Windows::Forms::Label ^ label4;
      private:System::Windows::Forms::NumericUpDown ^ Seconds;
      private:System::Windows::Forms::Label ^ label5;
      private:System::Windows::Forms::
            GroupBox ^ groupBoxProperties;
      private:System::Windows::Forms::Label ^ labelNFrames;
      private:System::Windows::Forms::Label ^ label6;
      private:System::Windows::Forms::Label ^ label7;
      private:System::Windows::Forms::Label ^ labelFPS;
      private:System::Windows::Forms::Timer ^ animTimer;
      private:System::ComponentModel::IContainer ^ components;
      private:System::Windows::Forms::Button ^ ExportButton;
      private:System::Windows::Forms::Button ^ LeftToRightAll;
      private:System::Windows::Forms::Button ^ button1;
      private:System::Windows::Forms::Label ^ label8;
      private:System::Windows::Forms::TextBox ^ ExeName;
      private:System::Windows::Forms::Button ^ BrowseButtonExe;
      private:System::Windows::Forms::
            FolderBrowserDialog ^ folderBrowserDialogExe;
      private:System::Windows::Forms::Label ^ label9;
      private:System::Windows::Forms::ComboBox ^ comboFileType;



      private:int iCurrFrame;

      private:
        /// <summary>
        /// Required designer variable.
        /// </summary>


#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void) {
            this->components = (gcnew System::ComponentModel::Container());
            System::Windows::Forms::GroupBox ^ groupBoxPreview;
            this->pictureBox1 =
                (gcnew System::Windows::Forms::PictureBox());
            this->saveFileDialog1 =
                (gcnew System::Windows::Forms::SaveFileDialog());
            this->BrowseButtonOut =
                (gcnew System::Windows::Forms::Button());
            this->FileNameOut = (gcnew System::Windows::Forms::TextBox());
            this->BrowseButtonIn =
                (gcnew System::Windows::Forms::Button());
            this->DirNameIn = (gcnew System::Windows::Forms::TextBox());
            this->folderBrowserDialog1 =
                (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->BrowseListBox =
                (gcnew System::Windows::Forms::ListBox());
            this->OkListBox = (gcnew System::Windows::Forms::ListBox());
            this->LeftToRight = (gcnew System::Windows::Forms::Button());
            this->RightToLeft = (gcnew System::Windows::Forms::Button());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->label4 = (gcnew System::Windows::Forms::Label());
            this->Seconds =
                (gcnew System::Windows::Forms::NumericUpDown());
            this->label5 = (gcnew System::Windows::Forms::Label());
            this->groupBoxProperties =
                (gcnew System::Windows::Forms::GroupBox());
            this->labelFPS = (gcnew System::Windows::Forms::Label());
            this->label7 = (gcnew System::Windows::Forms::Label());
            this->labelNFrames = (gcnew System::Windows::Forms::Label());
            this->label6 = (gcnew System::Windows::Forms::Label());
            this->animTimer =
                (gcnew System::Windows::Forms::Timer(this->components));
            this->ExportButton = (gcnew System::Windows::Forms::Button());
            this->LeftToRightAll =
                (gcnew System::Windows::Forms::Button());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->label8 = (gcnew System::Windows::Forms::Label());
            this->ExeName = (gcnew System::Windows::Forms::TextBox());
            this->BrowseButtonExe =
                (gcnew System::Windows::Forms::Button());
            this->folderBrowserDialogExe =
                (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->label9 = (gcnew System::Windows::Forms::Label());
            this->comboFileType =
                (gcnew System::Windows::Forms::ComboBox());
            groupBoxPreview = (gcnew System::Windows::Forms::GroupBox());
            groupBoxPreview->SuspendLayout();
            (cli::safe_cast <
             System::ComponentModel::ISupportInitialize ^ >(this->
                                                            pictureBox1))->
BeginInit();
            (cli::safe_cast <
             System::ComponentModel::ISupportInitialize ^ >(this->
                                                            Seconds))->
BeginInit();
            this->groupBoxProperties->SuspendLayout();
            this->SuspendLayout();
            // 
            // groupBoxPreview
            // 
            groupBoxPreview->Controls->Add(this->pictureBox1);
            groupBoxPreview->Location = System::Drawing::Point(269, 363);
            groupBoxPreview->Name = L"groupBoxPreview";
            groupBoxPreview->Size = System::Drawing::Size(210, 168);
            groupBoxPreview->TabIndex = 58;
            groupBoxPreview->TabStop = false;
            groupBoxPreview->Text = L"Preview :";
            // 
            // pictureBox1
            // 
            this->pictureBox1->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->pictureBox1->Location = System::Drawing::Point(6, 19);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(198, 143);
            this->pictureBox1->SizeMode =
                System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->pictureBox1->TabIndex = 7;
            this->pictureBox1->TabStop = false;
            // 
            // saveFileDialog1
            // 
            this->saveFileDialog1->DefaultExt = L"anim";
            this->saveFileDialog1->Filter =
                L"Anim files (*.anim)|*.anim|All files (*)|*";
            this->saveFileDialog1->Title = L"Save Anim file";
            // 
            // BrowseButtonOut
            // 
            this->BrowseButtonOut->BackColor =
                System::Drawing::Color::LightBlue;
            this->BrowseButtonOut->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->BrowseButtonOut->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->BrowseButtonOut->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->BrowseButtonOut->Location =
                System::Drawing::Point(451, 58);
            this->BrowseButtonOut->Name = L"BrowseButtonOut";
            this->BrowseButtonOut->Size = System::Drawing::Size(28, 28);
            this->BrowseButtonOut->TabIndex = 1;
            this->BrowseButtonOut->Text = L"...";
            this->BrowseButtonOut->UseVisualStyleBackColor = false;
            this->BrowseButtonOut->Click +=
                gcnew System::EventHandler(this,
                                           &Form1::OnClickedBrowseButton);
            // 
            // FileNameOut
            // 
            this->FileNameOut->BackColor =
                System::Drawing::Color::LightBlue;
            this->FileNameOut->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->FileNameOut->Location = System::Drawing::Point(12, 62);
            this->FileNameOut->Name = L"FileNameOut";
            this->FileNameOut->Size = System::Drawing::Size(433, 20);
            this->FileNameOut->TabIndex = 0;
            // 
            // BrowseButtonIn
            // 
            this->BrowseButtonIn->BackColor =
                System::Drawing::Color::LightBlue;
            this->BrowseButtonIn->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->BrowseButtonIn->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->BrowseButtonIn->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->BrowseButtonIn->Location =
                System::Drawing::Point(451, 102);
            this->BrowseButtonIn->Name = L"BrowseButtonIn";
            this->BrowseButtonIn->Size = System::Drawing::Size(28, 28);
            this->BrowseButtonIn->TabIndex = 3;
            this->BrowseButtonIn->Text = L"...";
            this->BrowseButtonIn->UseVisualStyleBackColor = true;
            this->BrowseButtonIn->Click +=
                gcnew System::EventHandler(this,
                                           &Form1::OnFileNameInClicked);
            // 
            // DirNameIn
            // 
            this->DirNameIn->BackColor = System::Drawing::Color::LightBlue;
            this->DirNameIn->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->DirNameIn->Location = System::Drawing::Point(12, 106);
            this->DirNameIn->Name = L"DirNameIn";
            this->DirNameIn->Size = System::Drawing::Size(433, 20);
            this->DirNameIn->TabIndex = 2;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(9, 45);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(58, 13);
            this->label1->TabIndex = 50;
            this->label1->Text = L"Output file:";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(9, 90);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(111, 13);
            this->label2->TabIndex = 50;
            this->label2->Text = L"Directory of input files:";
            // 
            // BrowseListBox
            // 
            this->BrowseListBox->BackColor =
                System::Drawing::Color::LightBlue;
            this->BrowseListBox->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->BrowseListBox->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->BrowseListBox->HorizontalScrollbar = true;
            this->BrowseListBox->Location =
                System::Drawing::Point(12, 186);
            this->BrowseListBox->Name = L"BrowseListBox";
            this->BrowseListBox->Size = System::Drawing::Size(210, 171);
            this->BrowseListBox->Sorted = true;
            this->BrowseListBox->TabIndex = 4;
            // 
            // OkListBox
            // 
            this->OkListBox->BackColor = System::Drawing::Color::LightBlue;
            this->OkListBox->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->OkListBox->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->OkListBox->HorizontalScrollbar = true;
            this->OkListBox->Location = System::Drawing::Point(269, 186);
            this->OkListBox->Name = L"OkListBox";
            this->OkListBox->Size = System::Drawing::Size(210, 171);
            this->OkListBox->TabIndex = 5;
            // 
            // LeftToRight
            // 
            this->LeftToRight->BackColor =
                System::Drawing::Color::LightBlue;
            this->LeftToRight->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->LeftToRight->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->LeftToRight->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->LeftToRight->Location = System::Drawing::Point(228, 227);
            this->LeftToRight->Name = L"LeftToRight";
            this->LeftToRight->Size = System::Drawing::Size(35, 35);
            this->LeftToRight->TabIndex = 6;
            this->LeftToRight->Text = L"->";
            this->LeftToRight->UseVisualStyleBackColor = false;
            this->LeftToRight->Click +=
                gcnew System::EventHandler(this, &Form1::OnToRightClick);
            // 
            // RightToLeft
            // 
            this->RightToLeft->BackColor =
                System::Drawing::Color::LightBlue;
            this->RightToLeft->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->RightToLeft->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->RightToLeft->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->RightToLeft->Location = System::Drawing::Point(228, 281);
            this->RightToLeft->Name = L"RightToLeft";
            this->RightToLeft->Size = System::Drawing::Size(35, 35);
            this->RightToLeft->TabIndex = 7;
            this->RightToLeft->Text = L"<-";
            this->RightToLeft->UseVisualStyleBackColor = false;
            this->RightToLeft->Click +=
                gcnew System::EventHandler(this, &Form1::OnToLeftClick);
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(9, 170);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(178, 13);
            this->label3->TabIndex = 53;
            this->label3->Text = L"Choose files to add to the animation:";
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Location = System::Drawing::Point(266, 170);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(142, 13);
            this->label4->TabIndex = 54;
            this->label4->Text = L"Current files in the animation:";
            // 
            // Seconds
            // 
            this->Seconds->BackColor = System::Drawing::Color::LightBlue;
            this->Seconds->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->Seconds->Cursor = System::Windows::Forms::Cursors::Hand;
            this->Seconds->Increment =
                System::Decimal(gcnew cli::array < System::Int32 > (4) {
                                10, 0, 0, 0});
            this->Seconds->Location = System::Drawing::Point(142, 21);
            this->Seconds->Maximum =
                System::Decimal(gcnew cli::array < System::Int32 > (4) {
                                268435455, 1042612833, 542101086, 0});
            this->Seconds->Name = L"Seconds";
            this->Seconds->Size = System::Drawing::Size(89, 20);
            this->Seconds->TabIndex = 8;
            this->Seconds->ThousandsSeparator = true;
            this->Seconds->Value =
                System::Decimal(gcnew cli::array < System::Int32 > (4) {
                                1000, 0, 0, 0});
            this->Seconds->ValueChanged +=
                gcnew System::EventHandler(this, &Form1::OnLengthChanged);
            // 
            // label5
            // 
            this->label5->AutoSize = true;
            this->label5->Location = System::Drawing::Point(6, 23);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(113, 13);
            this->label5->TabIndex = 56;
            this->label5->Text = L"Length in milliseconds:";
            // 
            // groupBoxProperties
            // 
            this->groupBoxProperties->Controls->Add(this->labelFPS);
            this->groupBoxProperties->Controls->Add(this->label7);
            this->groupBoxProperties->Controls->Add(this->labelNFrames);
            this->groupBoxProperties->Controls->Add(this->label6);
            this->groupBoxProperties->Controls->Add(this->label5);
            this->groupBoxProperties->Controls->Add(this->Seconds);
            this->groupBoxProperties->Location =
                System::Drawing::Point(12, 363);
            this->groupBoxProperties->Name = L"groupBoxProperties";
            this->groupBoxProperties->Size =
                System::Drawing::Size(237, 130);
            this->groupBoxProperties->TabIndex = 57;
            this->groupBoxProperties->TabStop = false;
            this->groupBoxProperties->Text = L"Properties :";
            // 
            // labelFPS
            // 
            this->labelFPS->AutoSize = true;
            this->labelFPS->Location = System::Drawing::Point(139, 90);
            this->labelFPS->Name = L"labelFPS";
            this->labelFPS->Size = System::Drawing::Size(13, 13);
            this->labelFPS->TabIndex = 60;
            this->labelFPS->Text = L"0";
            // 
            // label7
            // 
            this->label7->AutoSize = true;
            this->label7->Location = System::Drawing::Point(6, 90);
            this->label7->Name = L"label7";
            this->label7->Size = System::Drawing::Size(100, 13);
            this->label7->TabIndex = 59;
            this->label7->Text = L"Frames per second:";
            // 
            // labelNFrames
            // 
            this->labelNFrames->AutoSize = true;
            this->labelNFrames->Location = System::Drawing::Point(139, 58);
            this->labelNFrames->Name = L"labelNFrames";
            this->labelNFrames->Size = System::Drawing::Size(13, 13);
            this->labelNFrames->TabIndex = 58;
            this->labelNFrames->Text = L"0";
            // 
            // label6
            // 
            this->label6->AutoSize = true;
            this->label6->Location = System::Drawing::Point(6, 58);
            this->label6->Name = L"label6";
            this->label6->Size = System::Drawing::Size(99, 13);
            this->label6->TabIndex = 57;
            this->label6->Text = L"Number of pictures:";
            // 
            // animTimer
            // 
            this->animTimer->Enabled = true;
            this->animTimer->Tick +=
                gcnew System::EventHandler(this, &Form1::OnTimerTick);
            // 
            // ExportButton
            // 
            this->ExportButton->BackColor =
                System::Drawing::Color::LightBlue;
            this->ExportButton->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->ExportButton->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->ExportButton->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->ExportButton->Location = System::Drawing::Point(12, 499);
            this->ExportButton->Name = L"ExportButton";
            this->ExportButton->Size = System::Drawing::Size(236, 31);
            this->ExportButton->TabIndex = 59;
            this->ExportButton->Text = L"Export !";
            this->ExportButton->UseVisualStyleBackColor = false;
            this->ExportButton->Click +=
                gcnew System::EventHandler(this, &Form1::OnExport);
            // 
            // LeftToRightAll
            // 
            this->LeftToRightAll->BackColor =
                System::Drawing::Color::LightBlue;
            this->LeftToRightAll->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->LeftToRightAll->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->LeftToRightAll->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->LeftToRightAll->Location =
                System::Drawing::Point(228, 186);
            this->LeftToRightAll->Name = L"LeftToRightAll";
            this->LeftToRightAll->Size = System::Drawing::Size(35, 35);
            this->LeftToRightAll->TabIndex = 60;
            this->LeftToRightAll->Text = L"=>";
            this->LeftToRightAll->UseVisualStyleBackColor = false;
            this->LeftToRightAll->Click +=
                gcnew System::EventHandler(this, &Form1::OnLeftToRightAll);
            // 
            // button1
            // 
            this->button1->BackColor = System::Drawing::Color::LightBlue;
            this->button1->Cursor = System::Windows::Forms::Cursors::Hand;
            this->button1->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->button1->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->button1->Location = System::Drawing::Point(228, 322);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(35, 35);
            this->button1->TabIndex = 61;
            this->button1->Text = L"<=";
            this->button1->UseVisualStyleBackColor = false;
            this->button1->Click +=
                gcnew System::EventHandler(this, &Form1::OnRightToLeftAll);
            // 
            // label8
            // 
            this->label8->AutoSize = true;
            this->label8->Location = System::Drawing::Point(9, 5);
            this->label8->Name = L"label8";
            this->label8->Size = System::Drawing::Size(122, 13);
            this->label8->TabIndex = 64;
            this->label8->Text = L"FTS Packer executable:";
            // 
            // ExeName
            // 
            this->ExeName->BackColor = System::Drawing::Color::LightBlue;
            this->ExeName->BorderStyle =
                System::Windows::Forms::BorderStyle::FixedSingle;
            this->ExeName->Location = System::Drawing::Point(12, 22);
            this->ExeName->Name = L"ExeName";
            this->ExeName->Size = System::Drawing::Size(433, 20);
            this->ExeName->TabIndex = 62;
            // 
            // BrowseButtonExe
            // 
            this->BrowseButtonExe->BackColor =
                System::Drawing::Color::LightBlue;
            this->BrowseButtonExe->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->BrowseButtonExe->FlatAppearance->MouseOverBackColor =
                System::Drawing::Color::PowderBlue;
            this->BrowseButtonExe->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->BrowseButtonExe->Location =
                System::Drawing::Point(451, 18);
            this->BrowseButtonExe->Name = L"BrowseButtonExe";
            this->BrowseButtonExe->Size = System::Drawing::Size(28, 28);
            this->BrowseButtonExe->TabIndex = 63;
            this->BrowseButtonExe->Text = L"...";
            this->BrowseButtonExe->UseVisualStyleBackColor = false;
            this->BrowseButtonExe->Click +=
                gcnew System::EventHandler(this,
                                           &Form1::
                                           OnClickedBrowseButtonExe);
            // 
            // label9
            // 
            this->label9->AutoSize = true;
            this->label9->Location = System::Drawing::Point(9, 142);
            this->label9->Name = L"label9";
            this->label9->Size = System::Drawing::Size(370, 13);
            this->label9->TabIndex = 65;
            this->label9->Text =
                L"Choose here the file types you want to use, all files have to be the same type:";
            // 
            // comboFileType
            // 
            this->comboFileType->BackColor =
                System::Drawing::Color::LightBlue;
            this->comboFileType->Cursor =
                System::Windows::Forms::Cursors::Hand;
            this->comboFileType->FlatStyle =
                System::Windows::Forms::FlatStyle::Flat;
            this->comboFileType->FormattingEnabled = true;
            this->comboFileType->Items->AddRange(gcnew cli::array <
                                                 System::Object ^ >(3) {
                                                 L".png",
                                                 L".jpg and .jpeg",
                                                 L".bmp"});
            this->comboFileType->Location =
                System::Drawing::Point(380, 139);
            this->comboFileType->Name = L"comboFileType";
            this->comboFileType->Size = System::Drawing::Size(99, 21);
            this->comboFileType->TabIndex = 66;
            this->comboFileType->SelectedIndexChanged +=
                gcnew System::EventHandler(this,
                                           &Form1::OnSelChangedFileType);
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode =
                System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(491, 543);
            this->Controls->Add(this->comboFileType);
            this->Controls->Add(this->label9);
            this->Controls->Add(this->label8);
            this->Controls->Add(this->ExeName);
            this->Controls->Add(this->BrowseButtonExe);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->LeftToRightAll);
            this->Controls->Add(this->ExportButton);
            this->Controls->Add(groupBoxPreview);
            this->Controls->Add(this->groupBoxProperties);
            this->Controls->Add(this->label4);
            this->Controls->Add(this->label3);
            this->Controls->Add(this->RightToLeft);
            this->Controls->Add(this->LeftToRight);
            this->Controls->Add(this->OkListBox);
            this->Controls->Add(this->BrowseListBox);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->DirNameIn);
            this->Controls->Add(this->BrowseButtonIn);
            this->Controls->Add(this->FileNameOut);
            this->Controls->Add(this->BrowseButtonOut);
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"Form1";
            this->StartPosition =
                System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"Anim Maker";
            groupBoxPreview->ResumeLayout(false);
            (cli::safe_cast <
             System::ComponentModel::ISupportInitialize ^ >(this->
                                                            pictureBox1))->
EndInit();
            (cli::safe_cast <
             System::ComponentModel::ISupportInitialize ^ >(this->
                                                            Seconds))->
EndInit();
            this->groupBoxProperties->ResumeLayout(false);
            this->groupBoxProperties->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
      private:void DoUpdateFileList() {
            /* Add all possible files. */
            array < String ^ >^files;
            IEnumerator ^ efiles;
            this->BrowseListBox->Items->Clear();
            this->BrowseListBox->BeginUpdate();

            /* png. */
            if(this->comboFileType->SelectedIndex == 0) {
                files =
                    Directory::GetFiles(this->DirNameIn->Text, "*.png");
                efiles = files->GetEnumerator();
                while(efiles->MoveNext()) {
                    String ^ file =
                        safe_cast < String ^ >(efiles->Current);
                    array < String ^ >^nicename = file->Split('\\');
                    this->BrowseListBox->Items->Add(nicename->
                                                    GetValue(nicename->
                                                             Length - 1));
                }
            }

            /* jpg. */
            if(this->comboFileType->SelectedIndex == 1) {
                files =
                    Directory::GetFiles(this->DirNameIn->Text, "*.jp*g");
                efiles = files->GetEnumerator();
                while(efiles->MoveNext()) {
                    String ^ file =
                        safe_cast < String ^ >(efiles->Current);
                    array < String ^ >^nicename = file->Split('\\');
                    this->BrowseListBox->Items->Add(nicename->
                                                    GetValue(nicename->
                                                             Length - 1));
                }
            }

            /* bmp. */
            if(this->comboFileType->SelectedIndex == 2) {
                files =
                    Directory::GetFiles(this->DirNameIn->Text, "*.bmp");
                efiles = files->GetEnumerator();
                while(efiles->MoveNext()) {
                    String ^ file =
                        safe_cast < String ^ >(efiles->Current);
                    array < String ^ >^nicename = file->Split('\\');
                    this->BrowseListBox->Items->Add(nicename->
                                                    GetValue(nicename->
                                                             Length - 1));
                }
            }

            this->BrowseListBox->EndUpdate();
        }

      private:void DoUpdateDetails() {
            /* Avoid division by zero. */
            if(this->Seconds->Value == 0) {
                this->labelFPS->Text = "0";
                return;
            }

            /* Count the number of frames. */
            this->labelNFrames->Text =
                Convert::ToString(this->OkListBox->Items->Count);

            /* Calculate the FPS. */
            float FPS =
                (float)this->OkListBox->Items->Count /
                (float)this->Seconds->Value * 1000;
            this->labelFPS->Text = Convert::ToString(FPS);

            if(FPS != 0) {
                this->animTimer->Interval = (int)((1.0f / FPS) * 1000.0f);
            }
        }

      private:System::Void OnClickedBrowseButtonExe(System::Object ^ sender,
                                              System::
                                              EventArgs ^ e) {
            /* Let the user choose the place of the packer executable. */
            if(this->folderBrowserDialogExe->ShowDialog() ==
               Windows::Forms::DialogResult::OK) {
                this->ExeName->Text =
                    this->folderBrowserDialogExe->SelectedPath;
                if(!File::Exists(this->ExeName->Text + "\\" PACK_NAME)) {
                    this->ExeName->ForeColor = System::Drawing::Color::Red;
                } else {
                    this->ExeName->ForeColor =
                        System::Drawing::Color::Black;
                }
            }
        }

      private:System::Void OnFileNameInClicked(System::Object ^ sender,
                                         System::
                                         EventArgs ^ e) {
            /* Let the user choose a file name to save as. */
            if(this->folderBrowserDialog1->ShowDialog() ==
               Windows::Forms::DialogResult::OK) {
                this->DirNameIn->Text =
                    this->folderBrowserDialog1->SelectedPath;
                DoUpdateFileList();
            }
        }

      private:System::Void OnClickedBrowseButton(System::Object ^ sender,
                                           System::
                                           EventArgs ^ e) {
            /* Let the user browse to the input directory. */
            if(this->saveFileDialog1->ShowDialog() ==
               Windows::Forms::DialogResult::OK) {
                this->FileNameOut->Text = this->saveFileDialog1->FileName;
            }
        }

      private:System::Void OnToRightClick(System::Object ^ sender,
                                    System::EventArgs ^ e) {
            /* Add it to the right listbox */
            if(ListBox::NoMatches ==
               this->OkListBox->FindString(this->BrowseListBox->
                                           SelectedItem->ToString())
               && this->BrowseListBox->SelectedIndex >= 0) {
                this->OkListBox->Items->Add(this->BrowseListBox->
                                            SelectedItem);
            }

            /* Now disable the directory choosing edit+Button so there are only files from one directory. */
            this->DirNameIn->Enabled = false;
            this->BrowseButtonIn->Enabled = false;

            /* And update everything. */
            DoUpdateDetails();
        }
      private:System::Void OnToLeftClick(System::Object ^ sender,
                                   System::EventArgs ^ e) {
            /* Remove the selected itme from the right listbox. */
            if(this->OkListBox->SelectedIndex >= 0)
                this->OkListBox->Items->RemoveAt(this->OkListBox->
                                                 SelectedIndex);

            /* And update everything. */
            DoUpdateDetails();
        }
      private:System::Void OnLengthChanged(System::Object ^ sender,
                                     System::
                                     EventArgs ^ e) {
            /* When the length changed, update everything. */
            DoUpdateDetails();
        }
      private:System::Void OnTimerTick(System::Object ^ sender,
                                 System::EventArgs ^ e) {
            /* Avoid to do some shit :) */
            if(this->OkListBox->Items->Count == 0)
                return;

            /* Select the next picture. */
            this->iCurrFrame++;
            if(this->iCurrFrame >= this->OkListBox->Items->Count) {
                this->iCurrFrame = 0;
            }
            /* Load the picture, is only temp. */
            String ^ s = this->DirNameIn->Text;
            s += "\\" + this->OkListBox->Items[this->iCurrFrame];
            this->pictureBox1->Load(s);
        }

      private:System::Void OnLeftToRightAll(System::Object ^ sender,
                                      System::
                                      EventArgs ^ e) {
            /* Add everything to the right listbox */
            for(int i = 0; i < this->BrowseListBox->Items->Count; i++) {
                this->OkListBox->Items->Add(this->BrowseListBox->Items[i]);
            }

            /* And update everything. */
            DoUpdateDetails();
        }

      private:System::Void OnRightToLeftAll(System::Object ^ sender,
                                      System::
                                      EventArgs ^ e) {
            /* Remove everything from the right listbox */
            this->OkListBox->Items->Clear();

            /* And update everything. */
            DoUpdateDetails();
        }

      private:System::Void OnSelChangedFileType(System::Object ^ sender,
                                          System::
                                          EventArgs ^ e) {
            DoUpdateFileList();
        }

      private:System::Void OnExport(System::Object ^ sender,
                              System::EventArgs ^ e) {
            array < String ^, 2 > ^copyFiles;
            array < Boolean ^ >^doneFiles;
            int nFiles = this->OkListBox->Items->Count + 4;

            /* Let's begin to work ! (in the temp directory) */
            Environment::CurrentDirectory =
                Environment::GetEnvironmentVariable("TMP");

            /* Say we done no file yet. */
            doneFiles = gcnew array < Boolean ^ >(nFiles);
            for(int i = 0; i < nFiles; i++)
                doneFiles[i] = false;

            /* Collect all the files we need to copy. */
            copyFiles = gcnew array < String ^, 2 > (nFiles, 2);
            copyFiles[0, 0] = this->ExeName->Text + "\\" PACK_NAME;
            copyFiles[0, 1] =
                Environment::CurrentDirectory + "\\" + PACK_NAME;
            copyFiles[1, 0] = this->ExeName->Text + "\\archive1.dll";
            copyFiles[1, 1] =
                Environment::CurrentDirectory + "\\archive1.dll";
            copyFiles[2, 0] = this->ExeName->Text + "\\bzip2.dll";
            copyFiles[2, 1] =
                Environment::CurrentDirectory + "\\bzip2.dll";
            copyFiles[3, 0] = this->ExeName->Text + "\\zlib.dll";
            copyFiles[3, 1] = Environment::CurrentDirectory + "\\zlib.dll";

            /* Select the right extension for the pictures. */
            String ^ ext = "";
            switch (this->comboFileType->SelectedIndex) {
            case 0:
                ext = ".png";
                break;
            case 1:
                ext = ".jpg";
                break;
            case 2:
                ext = ".bmp";
                break;
            }

            /* Create the picture names [0][i][Ext] */
            for(int i = 0; i < this->OkListBox->Items->Count; i++) {
                copyFiles[i + 4, 0] =
                    this->DirNameIn->Text + "\\" +
                    this->OkListBox->Items[i];
                if(i < 9)
                    copyFiles[i + 4, 1] =
                        Environment::CurrentDirectory + "\\0" +
                        Convert::ToString(i + 1) + ext;
                else
                    copyFiles[i + 4, 1] =
                        Environment::CurrentDirectory + "\\" +
                        Convert::ToString(i + 1) + ext;
            }

            /* Copy 'em all ! */
            for(int i = 0; i < copyFiles->GetLength(0); i++) {
                if(!File::Exists(copyFiles[i, 0])) {
                    if(i < 4)
                        MessageBox::
                            Show
                            ("The packer executable or one of its dlls (archive.dll, bzip2.dll, zlib.dll) is not in the specified directory.");
                    else
                        MessageBox::
                            Show("Could not copy one of the pictures !");
                    continue;
                }

                try {
                    File::Copy(copyFiles[i, 0], copyFiles[i, 1], true);
                    doneFiles[i] = true;
                }
                catch(Exception ^ e) {
                    MessageBox::Show(e->Message);
                }
            }

            /* Create the anim's configuration file. */
            String ^ path = Environment::CurrentDirectory + "\\a.conf";
            String ^ content =
                "#This file was automatically created by anim_maker Utility for FTS.\n";
            switch (this->comboFileType->SelectedIndex) {
            case 0:
                content += "Ext    = \".png\"\n";
                break;
            case 1:
                content += "Ext    = \".jpg\"\n";
                break;
            case 2:
                content += "Ext    = \".bmp\"\n";
                break;
            }
            content += "Frames = " + this->labelNFrames->Text + "\n";
            content +=
                "Time   = " + Convert::ToString(this->Seconds->Value) +
                "\n";
            IO::File::WriteAllText(path, content);

            /* Build the command. */
            String ^ cmd =
                "\"" + Environment::CurrentDirectory + "\\" PACK_NAME "\"";
            String ^ args = " -c \"" + this->FileNameOut->Text + "\"";
            for(int i = 0; i < this->OkListBox->Items->Count; i++) {
                if(i < 9)
                    args +=
                        " \"" + "0" + Convert::ToString(i + 1) + ext +
                        "\"";
                else
                    args += " \"" + Convert::ToString(i + 1) + ext + "\"";
            }
            args += " \"a.conf\"";

            /* Execute it ! */
            Diagnostics::Process ^ proc =
                System::Diagnostics::Process::Start(cmd, args);

            while(!proc->HasExited) {
                System::Threading::Thread::Sleep(100);
            }

            /* And delete all temp files. */
            for(int i = 0; i < copyFiles->GetLength(0); i++) {
                if(doneFiles[i])
                    File::Delete(copyFiles[i, 1]);
            }
            File::Delete(Environment::CurrentDirectory + "\\a.conf");

            if(proc->ExitCode < 0) {
                String ^ errlogFile =
                    Environment::CurrentDirectory + "\\error.log";
                MessageBox::
                    Show
                    ("ERROR while creating the animation. See the followinf file for more info:\n"
                     + errlogFile);
            } else {
                MessageBox::Show("Animation successfully created !");
            }
        }
    };
}
