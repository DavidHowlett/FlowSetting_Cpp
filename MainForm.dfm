object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'flow setter'
  ClientHeight = 194
  ClientWidth = 501
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  DesignSize = (
    501
    194)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 120
    Height = 13
    Caption = 'current flow rate (SCCM)'
  end
  object Label2: TLabel
    Left = 8
    Top = 66
    Width = 112
    Height = 13
    Caption = 'target flow rate (SLPM)'
  end
  object Label3: TLabel
    Left = 144
    Top = 8
    Width = 45
    Height = 13
    Caption = 'Event log'
  end
  object SetFlow: TButton
    Left = 8
    Top = 112
    Width = 105
    Height = 25
    Caption = 'Set the flow'
    TabOrder = 0
  end
  object Button2: TButton
    Left = 8
    Top = 143
    Width = 105
    Height = 25
    Caption = 'Recalibrate system'
    TabOrder = 1
  end
  object MassFlowMonitor: TEdit
    Left = 8
    Top = 39
    Width = 121
    Height = 21
    TabOrder = 2
    Text = 'you should not see this'
  end
  object TargetMassFlow: TEdit
    Left = 8
    Top = 85
    Width = 121
    Height = 21
    TabOrder = 3
    Text = 'enter the flow rate here'
  end
  object Log: TListBox
    Left = 144
    Top = 39
    Width = 349
    Height = 147
    Anchors = [akLeft, akTop, akRight, akBottom]
    ItemHeight = 13
    TabOrder = 4
    ExplicitWidth = 513
    ExplicitHeight = 387
  end
  object ClearEventLog: TButton
    Left = 211
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Clear'
    TabOrder = 5
  end
end
