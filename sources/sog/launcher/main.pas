{
*********************************************************
* Name:          main.pas
* Description:   Main program SOG Launcher
* Project:       SOG Launcher
* Author:        Arno Ansems
* Last Modified: 1-25-2001
*********************************************************
}

unit main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, ShellAPI, ComCtrls;
const
  NUMBEROFMAPS = 80;
  MapTitles: array[0..NUMBEROFMAPS - 1,0..1] of String =
  (('sogdm1','Storage Room 37'),
   ('sogdm2','Tamara Powerplant'),
   ('sogdm3','House Of G|nter'),
   ('sogdm4','Secrets'),
   ('sogdm5','Golden'),
   ('sogdm6','Projectile Madnezz'),
   ('sogdm7','Ariq`s Towers'),
   ('sogdm8','Deimos Station'),
   ('sogdm9','Lair of the Black Templars'),
   ('sogdm10','Secrets of the Lost Temple'),
   ('sogdm11','Cataclyzm'),
   ('sogdm12','Revenge of the Space Pyrates'),
   ('sogdm13','Final Restingplace'),
   ('sogdm14','Cold War'),
   ('sogdm15','Darkened Hearts'),
   ('sogdm16','Dwelling Hatred'),
   ('sogctf1','Halls of Elrond'),
   ('sogctf2','Hillsides'),
   ('sog1','The Teleporter'),
   ('sog2','The Wastelands'),
   ('sog3','Caverns of Fear'),
   ('secret1','8-bit Nightmares'),
   ('sog4','The Forgery'),
   ('sog5','The Soul Harvestry'),
   ('sog6','The Circle'),
   ('sog7','City of Evil'),
   ('sog8','The Caconest'),
   ('sog9','Mouth of the Beast'),
   ('q2dm1','The Edge'),
   ('q2dm2','Tokay`s Towers'),
   ('q2dm3','The Frag Pipe'),
   ('q2dm4','Lost Hallways'),
   ('q2dm5','The Pits'),
   ('q2dm6','Lava Tomb'),
   ('q2dm7','The Slimy Place'),
   ('q2dm8','WareHouse'),
   ('base1','Outer Base'),
   ('base2','Installation'),
   ('base3','Comm Center'),
   ('train','Lost Station'),
   ('bunk1','Ammo Depot'),
   ('ware1','Supply Station'),
   ('ware2','Warehouse'),
   ('jail1','Main Gate'),
   ('jail2','Detention Center'),
   ('jail3','Security Complex'),
   ('jail4','Torture Chambers'),
   ('jail5','Guard House'),
   ('security','Grid Control'),
   ('mintro','Mine Entrance'),
   ('mine1','Upper Mines'),
   ('mine2','Bore Hole'),
   ('mine3','Drilling Area'),
   ('mine4','Lower Mines'),
   ('fact1','Receiving Center'),
   ('fact2','Processing Plant'),
   ('fact3','Sudden Death'),
   ('power1','Power Plant'),
   ('power2','The Reactor'),
   ('cool1','Cooling Facility'),
   ('waste1','Toxic Waste Dump'),
   ('waste2','Pumping Station 1'),
   ('waste3','Pumping Station 2'),
   ('biggun','Big Gun'),
   ('hangar1','Outer Hangar'),
   ('hanger2','Inner Hangar'),
   ('lab','Research Lab'),
   ('command','Launch Command'),
   ('strike','Outlands'),
   ('space','Comm Satellite'),
   ('city1','Outer Courts'),
   ('city2','Lower Palace'),
   ('city3','Upper Palace'),
   ('boss1','Inner Chamber'),
   ('boss2','Final Showdown'),
   ('q2ctf1','McKinley Revival'),
   ('q2ctf2','Stronghold Opposition'),
   ('q2ctf3','The Smelter'),
   ('q2ctf4','Outlands'),
   ('q2ctf5','Capture Showdown'));

type
  TForm1 = class(TForm)
    RadioGroup_GameMode: TRadioGroup;
    RadioGroup_ForceClass: TRadioGroup;
    GroupBox1: TGroupBox;
    CheckBox_DedServer: TCheckBox;
    CheckBox_UseKey: TCheckBox;
    CheckBox_FullyLoaded: TCheckBox;
    CheckBox_NoMapVoting: TCheckBox;
    GroupBox2: TGroupBox;
    ListBox_Bots: TListBox;
    Button1: TButton;
    ComboBox_AddBot: TComboBox;
    Button2: TButton;
    Button3: TButton;
    GroupBox3: TGroupBox;
    Edit_Map: TEdit;
    Label1: TLabel;
    Label_MapTitle: TLabel;
    Label3: TLabel;
    GroupBox4: TGroupBox;
    CheckBox_NoHealth: TCheckBox;
    CheckBox_NoItems: TCheckBox;
    CheckBox_WeaponsStay: TCheckBox;
    CheckBox_NoFalling: TCheckBox;
    CheckBox_InstantItems: TCheckBox;
    CheckBox_SameLevel: TCheckBox;
    CheckBox_SkinTeams: TCheckBox;
    CheckBox_ModelTeams: TCheckBox;
    CheckBox_NoFriendlyFire: TCheckBox;
    CheckBox_SpawnFarthest: TCheckBox;
    CheckBox_ForceRespawn: TCheckBox;
    CheckBox_NoArmor: TCheckBox;
    CheckBox_AllowExit: TCheckBox;
    CheckBox_InfiniteAmmo: TCheckBox;
    CheckBox_QuadDrop: TCheckBox;
    CheckBox_FixedFOV: TCheckBox;
    Edit_DMFlags: TEdit;
    Label4: TLabel;
    ComboBox_BotSkill: TComboBox;
    GroupBox5: TGroupBox;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    CheckBox_CTFBaseRespawn: TCheckBox;
    Edit_FragLimit: TEdit;
    Edit_CaptureLimit: TEdit;
    Edit_TimeLimit: TEdit;
    Label2: TLabel;
    Edit_ClientLimit: TEdit;
    Button4: TButton;
    procedure Initialize(Sender: TObject);
    procedure LaunchSOG(Sender: TObject);
    procedure AddBot(Sender: TObject);
    procedure RemoveBot(Sender: TObject);
    procedure CheckMapname(Sender: TObject);
    procedure CalculateDMFlags(Sender: TObject);
    procedure PlayStory(Sender: TObject);
  private
    procedure LoadBots;
    function Get_DMFlags: integer;
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  BotFile, BatchFile: Text;


implementation

{$R *.DFM}

{
====================
ExecuteFile

Executes a given program, in this case the batchfile with the
Quake2-commands.
====================
}

function ExecuteFile(const FileName, Params, DefaultDir: string;
  ShowCmd: Integer): THandle;
var
  zFileName, zParams, zDir: array[0..79] of Char;
begin
  Result := ShellExecute(Application.MainForm.Handle, nil,
    StrPCopy(zFileName, FileName), StrPCopy(zParams, Params),
    StrPCopy(zDir, DefaultDir), ShowCmd);
end;

{
====================
Int2Str

Coverts a integer to a string.
====================
}

Function Int2Str (const i: integer): string;
var s: string;
begin
  Str(i,s);
  Int2Str := s;
end;


{
====================
LoadBots

Loads all the bots names and settings from bots.cfg.
====================
}

procedure TForm1.LoadBots;
var s: string;
begin
  if FileExists('bots.cfg') then

    // Read Bots from file

    begin
      AssignFile(BotFile,'bots.cfg');
      Reset(BotFile);
      While NOT EoF(BotFile) do
        begin
          ReadLn (BotFile,s);
          if Length(s) > 0 then
            begin
              if NOT (s[1] in [' ','/']) then
                ComboBox_AddBot.Items.Add(s);
            end;
        end;
      CloseFile (BotFile);
    end
  else

    // Default Bots

    begin
      ComboBox_AddBot.Items.Add('Adam idg2/green red');
      ComboBox_AddBot.Items.Add('AklashPahk idg3/base red');
      ComboBox_AddBot.Items.Add('Ariq idg1/base blue');
      ComboBox_AddBot.Items.Add('Arno idg2/blue blue');
      ComboBox_AddBot.Items.Add('Chill idg2/red red');
      ComboBox_AddBot.Items.Add('DarkBishop male/major blue');
      ComboBox_AddBot.Items.Add('iGore idg2/white blue');
      ComboBox_AddBot.Items.Add('Kengbo idg3/brown blue');
      ComboBox_AddBot.Items.Add('Leed idg1/base red');
      ComboBox_AddBot.Items.Add('Railord idg3/brown red');
      ComboBox_AddBot.Items.Add('Scorpion idg3/base blue');
      ComboBox_AddBot.Items.Add('Skippy male/psycho red');
    end;
  ComboBox_AddBot.ItemIndex := 0;
end;

{
====================
Get_DMFlags

Reads all the checkboxes from the DM flags and
returns the corresponding integer.
====================
}

function TForm1.Get_DMFlags: integer;
var Flags: integer;
begin
  Flags := 0;
  if CheckBox_NoHealth.Checked then Inc(Flags,1);
  if CheckBox_NoItems.Checked then Inc(Flags,2);
  if CheckBox_WeaponsStay.Checked then Inc(Flags,4);
  if CheckBox_NoFalling.Checked then Inc(Flags,8);
  if CheckBox_InstantItems.Checked then Inc(Flags,16);
  if CheckBox_SameLevel.Checked then Inc(Flags,32);
  if CheckBox_SkinTeams.Checked then Inc(Flags,64);
  if CheckBox_ModelTeams.Checked then Inc(Flags,128);
  if CheckBox_NoFriendlyFire.Checked then Inc(Flags,256);
  if CheckBox_SpawnFarthest.Checked then Inc(Flags,512);
  if CheckBox_ForceRespawn.Checked then Inc(Flags,1024);
  if CheckBox_NoArmor.Checked then Inc(Flags,2048);
  if CheckBox_AllowExit.Checked then Inc(Flags,4096);
  if CheckBox_InfiniteAmmo.Checked then Inc(Flags,8192);
  if CheckBox_QuadDrop.Checked then Inc(Flags,16384);
  if CheckBox_FixedFOV.Checked then Inc(Flags,32768);
  Get_DMFlags := Flags;
end;


{
====================
Initialize

Sets the default values.
====================
}

procedure TForm1.Initialize(Sender: TObject);
begin
  RadioGroup_GameMode.ItemIndex := 2;
  RadioGroup_ForceClass.ItemIndex := 0;
  ComboBox_BotSkill.ItemIndex := 1;
  LoadBots;
end;


{
====================
LauchSOG

Checks all the settings in the launcher,
and then launches Quake II with all these settings.
====================
}

procedure TForm1.LaunchSOG(Sender: TObject);
var Parameters: string;
    NumParam: integer;
    SOGFlags, DMFlags: integer;
    Code, i: integer;
    Limit: integer;
begin
  Parameters := '';
  NumParam := 0;

  // Check for dedicated

  if CheckBox_DedServer.Checked then
    begin
      Parameters := Parameters + ' +set dedicated 1 ';
      Inc(NumParam);
    end;

  // Set the MOD

  Parameters := Parameters + '+set game sog';
  Inc(NumParam);

  // Check for Deathmatch-option

  if (RadioGroup_GameMode.ItemIndex >= 2) then
    begin
      Parameters := Parameters + ' +set deathmatch 1';
      Inc(NumParam);
    end;

  // Check for Coop-option

  if (RadioGroup_GameMode.ItemIndex = 1) then
    begin
      Parameters := Parameters + ' +set coop 1';
      Inc(NumParam);
    end;

  // Check for Team-option

  if (RadioGroup_GameMode.ItemIndex = 3) or
     (RadioGroup_GameMode.ItemIndex = 4) then
    begin
      Parameters := Parameters + ' +set sog_team 1';
      Inc(NumParam);
    end;

  // Check for CTF-option

  if RadioGroup_GameMode.ItemIndex = 5 then
    begin
      Parameters := Parameters + ' +set sog_ctf 1';
      Inc(NumParam);
    end;
  // Check for SOGFlags

  SOGFlags := 0;

  if RadioGroup_GameMode.ItemIndex = 4 then   // Team Red VS Blue
    SOGFlags := SOGFlags + 1;

  if RadioGroup_ForceClass.ItemIndex = 1 then   // Force Blaze
    SOGFlags := SOGFlags + 512;

  if RadioGroup_ForceClass.ItemIndex = 2 then   // Force Flynn
    SOGFlags := SOGFlags + 16;

  if RadioGroup_ForceClass.ItemIndex = 3 then   // Force Axe
    SOGFlags := SOGFlags + 8;

  if CheckBox_UseKey.Checked then          // Use Key
    SOGFlags := SOGFlags + 2;

  if CheckBox_FullyLoaded.Checked then          // Fully Loaded
    SOGFlags := SOGFlags + 4;

  if CheckBox_CTFBaseRespawn.Checked then     // CTF Base Respawn
    SOGFlags := SOGFlags + 32;

  if CheckBox_NoMapVoting.Checked then          // No Map Voting
    SOGFlags := SOGFlags + 64;

  if SOGFlags <> 0 then
    begin
      Parameters := Parameters + ' +set sogflags ' + Int2Str(SOGFlags);
      Inc(NumParam);
    end;

  // Limits

  Val(Edit_FragLimit.Text,Limit,Code);
  if ((Code = 0) AND (Limit > 0)) then
    begin
      Parameters := Parameters + ' +set fraglimit ' + Int2Str(Limit);
      Inc(NumParam);
    end;

  Val(Edit_CaptureLimit.Text,Limit,Code);
  if ((Code = 0) AND (Limit > 0)) then
    begin
      Parameters := Parameters + ' +set limit ' + Int2Str(Limit);
      Inc(NumParam);
    end;

  Val(Edit_TimeLimit.Text,Limit,Code);
  if ((Code = 0) AND (Limit > 0)) then
    begin
      Parameters := Parameters + ' +set timelimit ' + Int2Str(Limit);
      Inc(NumParam);
    end;

  Val(Edit_ClientLimit.Text,Limit,Code);
  if ((Code = 0) AND (Limit > 0)) then
    begin
      Parameters := Parameters + ' +set maxclients ' + Int2Str(Limit);
      Inc(NumParam);
    end;

  // Deathmatch Flags

  Val(Edit_DMFlags.Text,DMFlags,Code);
  if ((Code = 0) AND (DMFlags > 0)) then
    begin
      Parameters := Parameters + ' +set dmflags ' + Int2Str(DMFlags);
      Inc(NumParam);
    end;

  // Skill settings

  if ComboBox_BotSkill.ItemIndex = 0 then
    Parameters := Parameters + ' +set skill 0 '
  else if ComboBox_BotSkill.ItemIndex = 1 then
    Parameters := Parameters + ' +set skill 1 '
  else if ComboBox_BotSkill.ItemIndex = 2 then
    Parameters := Parameters + ' +set skill 2 '
  else if ComboBox_BotSkill.ItemIndex = 3 then
    Parameters := Parameters + ' +set skill 3 ';
  Inc(NumParam);

  // Map

  if Edit_Map.Text <> '' then
    begin
      Parameters := Parameters + ' +map ' + Edit_Map.Text;
      Inc(NumParam);
    end;

  // Add bots

  if ListBox_Bots.Items.Count <> 0 then
    begin
      if (RadioGroup_GameMode.ItemIndex = 1) then
        begin
          MessageDlg('Bots are not allowed in cooperative-mode !' + chr(13) + 'Remove the bots from the bot-display.',mtWarning,[mbOK],0);
          Exit;
        end
      else if RadioGroup_GameMode.ItemIndex = 5 then
        begin
          If MessageDlg('It is highly unrecommended to use bots in a CTF match !'
          + chr(13) + 'The bot A.I. is insufficient and the game might become unstable.'
          + chr(13) + 'Proceed anyway?'
          ,mtWarning,[mbYes,mbNo],0) <> mrYes then Exit;
        end;
      For i := 0 to ListBox_Bots.Items.Count - 1 do
        begin
          Parameters := Parameters + ' +sv addbot ' + ListBox_Bots.Items[i];
          Inc(NumParam);
        end;
    end;

  If NumParam > 11 then
    begin
      MessageDlg('You exceeded the maximum number of commandline parameters !' + chr(13) + 'Quake II allows only 11 commandline parameters simultaniously.'
      + chr(13) + 'Remove some bots to decrease the number of parameters.'
      + chr(13) + 'You can always add more bots ingame, with the SV ADDBOT command.',mtWarning,[mbOK],0);
      Exit;
    end;


  // Create Batch-file

  AssignFile(BatchFile,'launch.bat');
  ReWrite(BatchFile);
  WriteLn(BatchFile,'del bots.tmp');
  WriteLn(BatchFile,'cd..');
  WriteLn(BatchFile,'quake2.exe ' + Parameters);
  CloseFile(BatchFile);

  // Run Batch-file

  ExecuteFile('launch.bat','','',SW_SHOW);
end;

{
====================
AddBot

Takes the selected bot from the combobox,
and adds it to the bot list.
====================
}

procedure TForm1.AddBot(Sender: TObject);
var NotPresent: boolean;
    i: integer;
begin
  if (ComboBox_AddBot.ItemIndex < 0) or (ComboBox_AddBot.ItemIndex > ComboBox_AddBot.Items.Count) then Exit;
  if ComboBox_AddBot.Items[ComboBox_AddBot.ItemIndex] <> '' then
    begin
      NotPresent := True;
      if ListBox_Bots.Items.Count > 0 then
        begin
          For i := 0 to ListBox_Bots.Items.Count - 1 do
            if ComboBox_AddBot.Items[ComboBox_AddBot.ItemIndex] = ListBox_Bots.Items[i] then
              NotPresent := False;
        end;
      if NotPresent then
        begin
          ListBox_Bots.Items.Add(ComboBox_AddBot.Items[ComboBox_AddBot.ItemIndex]);
          ComboBox_AddBot.Items.Delete(ComboBox_Addbot.ItemIndex);
          ComboBox_AddBot.ItemIndex := 0;
        end;
    end;
end;

{
====================
RemoveBot

Removes the selected bot from the bot list,
and puts it back into the combobox.
====================
}

procedure TForm1.RemoveBot(Sender: TObject);
begin
  if (ListBox_Bots.ItemIndex < 0) or (ListBox_Bots.ItemIndex > ListBox_Bots.Items.Count) then Exit;
    begin
      ComboBox_AddBot.Items.Add(ListBox_Bots.Items[ListBox_Bots.ItemIndex]);
      ListBox_Bots.Items.Delete(ListBox_Bots.ItemIndex);
      ListBox_Bots.ItemIndex := 0;
      if ComboBox_AddBot.Items.Count = 1 then ComboBox_AddBot.ItemIndex := 0;
    end;
end;

{
====================
CheckMapname

Checks if the submitted map name resembles a name from
the internal map list.
====================
}

procedure TForm1.CheckMapname(Sender: TObject);
var i: integer;
    MapNumber: integer;
begin
  MapNumber := -1;
  For i := 0 to NUMBEROFMAPS - 1 do
    if Edit_Map.Text = MapTitles[i,0] then MapNumber := i;
  if MapNumber = -1 then Label_MapTitle.Caption := 'User Map'
  else Label_MapTitle.Caption := MapTitles[MapNumber,1];
end;

{
====================
CalculateDMFlags

Displays the total value of the DM flags on the screen,
based on the checkboxes above.
====================
}

procedure TForm1.CalculateDMFlags(Sender: TObject);
begin
  Edit_DMFlags.Text := Int2Str(Get_DMFlags);
end;

{
====================
PlayStory

Loads SOG with the single player storyline.
====================
}

procedure TForm1.PlayStory(Sender: TObject);
var Parameters: string;
begin
  Parameters := '';

  // Set the MOD
  Parameters := Parameters + '+set game sog';

  // Check for Coop-option

  if (RadioGroup_GameMode.ItemIndex = 1) then
    Parameters := Parameters + ' +set coop 1';

  // Skill settings

  if ComboBox_BotSkill.ItemIndex = 0 then
    Parameters := Parameters + ' +set skill 0 '
  else if ComboBox_BotSkill.ItemIndex = 1 then
    Parameters := Parameters + ' +set skill 1 '
  else if ComboBox_BotSkill.ItemIndex = 2 then
    Parameters := Parameters + ' +set skill 2 '
  else if ComboBox_BotSkill.ItemIndex = 3 then
    Parameters := Parameters + ' +set skill 3 ';

  // Set the map

  Parameters := Parameters + ' +map sog0';

  // Launch Quake II

  ExecuteFile('quake2.exe',Parameters,'../',SW_SHOW);
end;

end.
