#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// I think this requires a C++ compile...

  double str_adjust, rst_adjust;	// maximum adjustment...
  double ping;
  double f, result, final;
  int maxping;
  FILE *F;
  char s[81];
  int bsr, satisfied;

int main();
void do_intro();
void open_file();
void close_file();
void ask_questions();
int gamemode();
void pingmax();
void pingmin();
void laghelp_str();
void laghelp_rst();
void lagpeak();
void lagtimeout();
void bopsickremove();
void lagbare();
void lagsick();
void bopmanageobservers();
void lagpent();
void laginvisible();
void lagslag();
void lagsuicide();
void lagverbose();
void nobophelp();
void list_pings(float);
void do_charts();
void clear();

int main() {

  do_intro();
  open_file();
  ask_questions();
  close_file();

}

void do_intro() {

  clear();
  cout << "Welcome to the BOP.INI file creator" << endl << endl;
  cout << "Written by Matt Ownby (bop@planetquake.com)" << endl << endl;
  cout << "This will overwrite any BOP.INI file in the current path" << endl;
  cout << "THIS SHOULD ONLY BE USED ON BOP v0.92 AND ABOVE!!\n\n";

}

void open_file() {

  F = fopen("bop.ini", "w");
  if (!F)
	cout << "Error creating the BOP.INI file!!\n";

}

void close_file() {

  cout << "This concludes the BOP.INI file creation process.\n";

  fclose(F);

}

void ask_questions() {

  if (!gamemode()) {
    satisfied = 0;
    while (!satisfied) {
      pingmax();
      pingmin();
      laghelp_str();
      laghelp_rst();
      do_charts();
    }
  }
  lagpeak();
  lagtimeout();
  lagbare();
  bopsickremove();
  lagpent();
  lagsick();
  bopmanageobservers();
  laginvisible();
  lagverbose();
  lagslag();
  lagsuicide();
  nobophelp();

}

int gamemode() {

  char ch;

  cout << "Balance of Power has 3 preset Game Modes built in." << endl;
  cout << "0 - Normal Mode	[Moderate    Lag Adjustment]\n";
  cout << "1 - Expert Mode	[Minimal     Lag Adjustment]\n";
  cout << "2 - Tournament Mode	[Phone-Jack Only Protection]\n\n";
  cout << "Choose (0-2) or any other key to enter in your own values: ";
  cin >> ch;
  if (ch<'0' || ch>'2')
	return(0);
  fprintf(F, "gamemode %c\n", ch);
  return(1);

}

void pingmax() {

  clear();
  cout << "Reaper[DTH] is an LPB with a ping of 50.  Whane[MEAT] is\n";
  cout << "an HPB with a ping of 450.  The differences in their\n";
  cout << "pings is 400.  Or in other words, their delta ping is 400.\n";
  cout << "LaggedLarry's ping is 2000.  His delta ping in relation to\n";
  cout << "Reaper[DTH] is 1950.  That is a lot of lag adjustment!\n";
  cout << "Maybe we should say only delta pings of up to 1000 can\n";
  cout << "qualify for lag adjustment and anything beyond is counted\n";
  cout << "as if it is 1000.  What do you think?\n\n";

  cout << "Enter in the maximum delta ping that you will allow to\n";
  cout << "receive lag adjustments (Example 1000) : ";
  cin >> s;
  fprintf(F, "pingmax %s\n", s);
  maxping = atoi(s);

}

void pingmin() {

  cout << "How far apart (in pings) do two players have to be before\n";
  cout << "one of them gets lag adjustment against the other? (example 100): ";
  cin >> s;
  fprintf(F, "pingmin %s\n", s);

}

void laghelp_str() {

  cout << "A maximum strength adjustment of 1.25 would mean an HPB\n";
  cout << "could AT BEST do 1.25 times his normal damage.\n";
  cout << "What is the maximum strength adjustment you will allow an\n";
  cout << "HPB to achieve? ";
  cin >> s;
  str_adjust = atof(s) - 1;
  fprintf(F, "laghelp_str %0.2f\n", str_adjust);

}

void laghelp_rst() {

  cout << "A maximum resistance adjustment of 1.25 would mean an HPB\n";
  cout << "could AT BEST receive his regular damage divided by 1.25\n";
  cout << "What is the maximum resistance adjustment you will allow an\n";
  cout << "HPB to achieve? ";
  cin >> s;
  rst_adjust = atof(s) - 1;
  fprintf(F, "laghelp_rst %0.2f\n", rst_adjust);

}

void lagpeak() {

  cout << "At what ping level will you cause HPB's to receive lag\n";
  cout << "sickness? (Example 1000) : ";
  cin >> s;
  fprintf(F, "lagpeak %s\n", s);

}

void lagtimeout() {

  cout << "How many seconds of packet loss before HPB's get lag sickness?\n";
  cout << "(Example 3) : ";
  cin >> s;
  fprintf(F, "lagtimeout %d\n", (atoi(s)*10));

}

void lagbare() {

  cout << "How many seconds before players can get lag sickness again?\n";
  cout << "(Example 60) : ";
  cin >> s;
  fprintf(F, "lagbare %s\n", s);

}

void bopsickremove() {

  cout << "BoP has two ways of dealing with frozen clients.\n\n";
  cout << "1 - The newest (and best) is to remove the lagged players\n";
  cout << "from the game temporarily and put them back in once they\n";
  cout << "aren't frozen anymore.  This is definitely the best one to\n";
  cout << "use, but it may cause conflicts with certain mods.\n";
  cout << "Give it a try and hopefully it will work for you.\n";
  cout << "0 - The older way is to give lagged players invulnerability\n";
  cout << "and keep them in the game.  This will no doubt be\n";
  cout << "the more reliable of the two, but it is likely to\n";
  cout << "make your LPB's upset.  Use it if you have to...\n";
  cout << "\nChoose 1 or 0 : ";
  cin >> s;
  bsr = atoi(s);
  fprintf(F, "bopsickremove %d\n", bsr);

}

void bopmanageobservers() {

  cout << "BoP uses a special technique for temporarily removing players\n";
  cout << "from the game by turning them into observers.\n";
  cout << "Due to backdoors, it is possible to abuse this feature\n";
  cout << "and turn into an invisible man who can shoot.  BoP can safeguard\n";
  cout << "against this by managing all observers in the game.\n";
  cout << "BoP should be smart enough to detect when legitimate observers\n";
  cout << "are intended to be in the game (such as in CTF, for example).\n";
  cout << "It is recommended that you use this feature!  If you run into\n";
  cout << "problems with BoP interfering with your legit observers,\n";
  cout << "then disable this feature and report bugs to me =)\n";
  cout << "Press 1 to use this feature, 0 to disable it : ";

  cin >> s;
  fprintf(F, "bopmanageobservers %s\n", s);

}

void lagsick() {

  cout << "The best situation is to have lag sickness not last very\n";
  cout << "long once the frozen player is back in the game.\n";
  cout << "If people are abusing lag sickness to become invulnerable,\n";
  cout << "you may need to increase this number.\n";
  cout << "How long should lag sickness last after invulnerability\n";
  cout << "wears off?\n";
  if (bsr)
    cout << "(Try 1 second) : ";
  else
    cout << "(Example, 5 seconds) : ";

  cin >> s;

  fprintf(F, "lagsick %s\n", s);

}

void lagpent() {

  if (bsr) {
    cout << "It is a good idea to keep frozen clients out of the game\n";
    cout << "for a short time lest their leaving/entering annoy other players.\n";
    cout << "How many seconds should frozen clients be out of the\n";
    cout << "action once they unfreeze? (Example 4) : ";
  }
  else {
	cout << "How many seconds of invulnerability do you want lag\n";
	cout << "sick players to have? (Example 10) : ";
  }
  cin >> s;
  fprintf(F, "lagpent %s\n", s);

}

void laginvisible() {

  cout << "BoP can run in one of two ways:\n";
  cout << "0 - Visible mode.  Does blue & red flashing effects\n";
  cout << "and gives lag sickness alerts\n";
  cout << "1 - Invisible mode (recommended).  Tries to appear\n";
  cout << "like an ordinary server without BoP.  Tries to be\n";
  cout << "unobtrusive.  This one is recommended.";
  cout << "Choose 0 of 1 : ";
  cin >> s;
  fprintf(F, "laginvisible %s\n", s); 

}

void lagslag() {

  cout << "If frozen players get hurt by external forces, should\n";
  cout << "they get lag sickness?  Enter 1 if you say yes : ";
  cin >> s;
  fprintf(F, "lagslag %s\n", s);

}

void lagsuicide() {

  cout << "If frozen players hurt themselves (by firing a rocket into\n";
  cout << "a wall, for example), should they get lag sickness?\n";
  cout << "Enter 1 if you say yes : ";
  cin >> s;
  fprintf(F, "lagsuicide %s\n", s);

}

void lagverbose() {

  cout << "BoP can make lots of messages detailing all the lag\n";
  cout << "compensation that takes place.  This is very useful\n";
  cout << "for testing out new configurations (but you won't\n";
  cout << "want to use this once you're ready to begin hosting\n";
  cout << "users on your server).\n";
  cout << "Enter 1 to turn on verbose messaging : ";
  cin >> s;
  fprintf(F, "lagverbose %d\n", atoi(s));

}

void nobophelp() {

  cout << "\nBoP has a help menu which gives users the current\n";
  cout << "BoP configuration.  You can prevent users from using\n";
  cout << "this menu if you wish.\n";
  cout << "Type 0 to let them use the menu (recommended)\n";
  cout << "Type 1 to prevent them from menu : ";
  cin >> s;
  fprintf(F, "nobophelp %s\n", s);

}

void do_charts() {

  char ch;

  clear();
  cout << "Here's how your adjustments look for strength:\n\n";
  list_pings(str_adjust);
  cout << "\nType a key and hit enter (stupid C++ hehe) : ";
  cin >> s;

  clear();
  cout << "Here's how your adjustments look for resistance:\n\n";
  list_pings(rst_adjust);
  cout << "\nAre you satisfied with this?  1 is Yes, 0 is Try Again : ";
  cin >> ch;

  if (ch=='1')
	satisfied = 1;

}

void list_pings(float adjust) {

  cout << "PI/2 (90 degrees) is equal to " << (PI/2) << endl;

  for (ping = 0; ping<maxping; ping+=(maxping/10)) {

    f = (ping/maxping)*(PI/2);
    result = sin(f);
    final = 1+(result*adjust);
    cout << "Delta Ping is " << ping << " - X is " << f << " - Sin(x) is ";
    cout << result << " - Adjustment is " << final << endl;

  }

}

void clear() {

  for (int i = 0; i<24; i++)
	cout << "\n";

}
