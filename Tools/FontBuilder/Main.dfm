object MainForm: TMainForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Font Builder'
  ClientHeight = 366
  ClientWidth = 202
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBoxTop: TGroupBox
    Left = 8
    Top = 8
    Width = 185
    Height = 81
    TabOrder = 0
    object ButtonPick: TButton
      Left = 9
      Top = 8
      Width = 81
      Height = 25
      Caption = 'Pick Font...'
      TabOrder = 0
      OnClick = ButtonPickClick
    end
    object ButtonMake: TButton
      Left = 9
      Top = 39
      Width = 161
      Height = 25
      Caption = 'Make'
      TabOrder = 1
      OnClick = ButtonMakeClick
    end
    object CheckSmooth: TCheckBox
      Left = 112
      Top = 12
      Width = 58
      Height = 17
      Alignment = taLeftJustify
      Caption = 'Smooth?'
      TabOrder = 2
    end
  end
  object GroupBoxTest: TGroupBox
    Left = 8
    Top = 95
    Width = 185
    Height = 263
    Caption = 'Tahoma8'
    TabOrder = 1
    object Memo: TMemo
      Left = 2
      Top = 15
      Width = 181
      Height = 246
      Align = alClient
      Lines.Strings = (
        'ABCDEFGHIJKLMNOPQRSTUVW'
        'XYZabcdefghijklmnopqrstuvwxy'
        'z1234567890/*-+~!@#$'
        '%^&*()_={}[]'
        #1040#1041#1042#1043#1044#1045#1025#1046#1047#1048#1049#1050#1051#1052#1053#1054#1055#1056#1057#1058#1059
        #1060#1061#1062#1063#1064#1065#1068#1067#1066#1069#1070#1071#1072#1073#1074#1075#1076#1077#1105#1078#1079
        #1080#1081#1082#1083#1084#1085#1086#1087#1088#1089#1090#1091#1092#1093#1094#1095#1096#1097#1100#1099#1098#1101#1102
        #1103'.,?":;<>'#39#1030#1111#1110)
      ScrollBars = ssVertical
      TabOrder = 0
    end
  end
  object FontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Left = 152
    Top = 56
  end
end
