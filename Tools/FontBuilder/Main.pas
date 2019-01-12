(*=============================================================================
    Main.pas: Font builder main file.
    Copyright 2015-2016 Vlad Gordienko.
=============================================================================*)
unit Main;
interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs,
  Vcl.StdCtrls, Math;

type
  TMainForm = class(TForm)
    ButtonPick: TButton;
    Memo: TMemo;
    CheckSmooth: TCheckBox;
    ButtonMake: TButton;
    GroupBoxTop: TGroupBox;
    GroupBoxTest: TGroupBox;
    FontDialog: TFontDialog;
    procedure ButtonPickClick(Sender: TObject);
    procedure ButtonMakeClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  MainForm: TMainForm;

implementation

{$R *.dfm}


function RemoveWhitespace( S: String ): String;
var
    iSpace: Integer;
begin
    Result := S;
    while True do
    begin
        iSpace := Pos( ' ', Result );
        if iSpace = 0 then Break;
        Delete( Result, iSpace, 1 );
    end;
end;


//
// Make this font.
//
procedure TMainForm.ButtonMakeClick( Sender: TObject );
var
    OutFile: TextFile;
    Bitmap: TBitmap;
    X, Y, iBitmap, i: Integer;
    CharSize, MaxSize: TSize;
    C: Char;
begin
    AssignFile( OutFile, Format('%s%d.%s',
                         [RemoveWhitespace(FontDialog.Font.Name), Abs(FontDialog.Font.Height), 'flf']) );
    Rewrite(OutFile);
    begin
        // Write font height and name.
        WriteLn( OutFile, Abs(FontDialog.Font.Height), ' ', RemoveWhitespace(FontDialog.Font.Name) );

        // Allocate bitmap for drawing.
        Bitmap                      := TBitmap.Create();
        Bitmap.Width                := 256;
        Bitmap.Height               := 256;
        Bitmap.PixelFormat          := pf32bit;
        Bitmap.Canvas.Pen.Style     := psClear;
        Bitmap.Canvas.Brush.Color   := clBlack;
        Bitmap.Canvas.Font          := FontDialog.Font;
        Bitmap.Canvas.Font.Color    := clWhite;
        Bitmap.Canvas.Rectangle( 0, 0, 257, 257 );
        if CheckSmooth.Checked then
            Bitmap.Canvas.Font.Quality := fqAntialiased
        else
            Bitmap.Canvas.Font.Quality := fqDraft;

        // Compute maximum size of character.
        MaxSize.cx := 0;
        MaxSize.cy := 0;
        for i := 1 to Length(Memo.Text) do
        begin
            C := Memo.Text[i];
            GetTextExtentPoint32( Bitmap.Canvas.Handle, C, 1, CharSize );
            MaxSize.cx := Max( MaxSize.cx, CharSize.cx );
            MaxSize.cy := Max( MaxSize.cy, CharSize.cy );
        end;

        // Process each character.
        iBitmap := 0;
        X       := 0;
        Y       := 0;

        for i := 1 to Length(Memo.Text) do
        begin
            C := Memo.Text[i];
            GetTextExtentPoint32( Bitmap.Canvas.Handle, C, 1, CharSize );

            // Check for valid character.
            if Ord(C) <= 32 then
                Continue;

            // May-be doenst fit in line.
            if ( X + MaxSize.cx ) >= 255 then
            begin
                X := 0;
                Y := Y + MaxSize.cy;
             end;

            // May-be doesnt fit in page.
            if ( Y + MaxSize.cy ) >= 255 then
            begin
                X := 0;
                Y := 0;

                Bitmap.SaveToFile(Format('%s%d_%d.bmp',
                    [RemoveWhitespace(Bitmap.Canvas.Font.Name), Abs(Bitmap.Canvas.Font.Size), iBitmap]));

                Bitmap.Canvas.Rectangle( 0, 0, 257, 257 );
                iBitmap := iBitmap + 1;
            end;

            // Render.
            Bitmap.Canvas.TextOut( X, Y, C );
            WriteLn( OutFile, Format( '%s %d %d %d %d %d', [C, iBitmap, X, Y, CharSize.cx, CharSize.cy] ) );

            X := X + MaxSize.cx;
        end;

        Bitmap.SaveToFile(Format('%s%d_%d.bmp',
              [RemoveWhitespace(Bitmap.Canvas.Font.Name), Abs(Bitmap.Canvas.Font.Height), iBitmap]));
    end;
    CloseFile(OutFile);

    MessageBox( 0, 'Font created', 'Builder', MB_OK or MB_ICONINFORMATION );
end;


//
// Pick a font.
//
procedure TMainForm.ButtonPickClick( Sender: TObject );
begin
    if FontDialog.Execute() then
    begin
        Memo.Font := FontDialog.Font;
        GroupBoxTest.Caption := FontDialog.Font.Name;
    end;
end;


end.
(*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*)