// Copyright 1999 by Christopher Gabriel Sisk, aka RipVTide

void JustTalk(edict_t *ent, edict_t *activator);
int loadconversation(char *filename);
void clearglobals();
char *ConversationRedraw();
char *MakeMeStrings(char *original_string, int wanted_strlngth, int string_request);
void givemequotes(char *string);
void givemequotes2();
void IChooseYou();
void IChooseYou2();