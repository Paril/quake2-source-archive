
#include "g_local.h"
#include "m_sheep.h"

void sheep_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	VectorSubtract (self->s.origin, other->s.origin, v);
        self->s.angles[YAW] = vectoyaw(v);
        self->count = 2;
        self->s.frame = FRAME_trot1;
}

void sheep_think (edict_t *self)
{
        // move sheep if trotting
        if (self->count == 2)
                M_walkmove (self, self->s.angles[YAW], 6);

        if (!self->count && (self->s.frame >= FRAME_grazeA40))
                self->s.frame = FRAME_grazeA1;
        if ((self->count == 1) && (self->s.frame >= FRAME_grazeB40))
                self->s.frame = FRAME_grazeB1;
        if ((self->count == 2) && (self->s.frame >= FRAME_trot8))
                self->count = (int)random();

        self->s.frame++;

        // occasionally change the grazing animation or wander off
        if ((self->count != 2) && (random() < 0.01))
        {
                if (random() < 0.5)
                {
                        self->s.angles[YAW] += random()*180-90;
                        self->count = 2;
                        self->s.frame = FRAME_trot1;
                }
                else if (!self->count)
                {
                        self->count = 1;
                        self->s.frame = FRAME_grazeB1;
                }
                else
                {
                        self->count = 0;
                        self->s.frame = FRAME_grazeA1;
                }
        }

        // occaisionally make a noise
        if (random() < 0.02)
        {
                int     r;

                r = random()*2;
                switch (r)
                {
                case 0:
                        gi.sound(self, CHAN_AUTO, gi.soundindex("misc/sheep1.wav"), 1, ATTN_NORM, 0);
                        break;
                case 1:
                        gi.sound(self, CHAN_AUTO, gi.soundindex("misc/sheep2.wav"), 1, ATTN_NORM, 0);
                        break;
                case 3:
                        gi.sound(self, CHAN_AUTO, gi.soundindex("misc/sheep3.wav"), 1, ATTN_NORM, 0);
                        break;
                }
        }

        self->nextthink = level.time + FRAMETIME;
}

void sheep_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

        self->pain_debounce_time = level.time + 3;
        gi.sound (self, CHAN_AUTO, gi.soundindex ("misc/sheepdie.wav"), 1, ATTN_NORM, 0);
}

void sheep_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        int     n;

        gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n= 0; n < 2; n++)
                ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
        for (n= 0; n < 4; n++)
                ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);       
        self->deadflag = DEAD_DEAD;
        self->nextthink = 0;
        self->s.modelindex = 0;
        self->s.effects = 0;
        gi.linkentity(self);
        G_FreeEdict(self);
}

void sheep_create (edict_t *self)
{
        self->movetype = MOVETYPE_STEP;
        self->solid = SOLID_BBOX;
        self->s.modelindex = gi.modelindex("models/misc/sheep/tris.md2");
        VectorSet(self->maxs, 20, 10, 30);
        VectorSet(self->mins, -20, -10, 0);

        self->health = 100;
        self->gib_health = -1;
        self->mass = 200;

        self->pain = sheep_pain;
        self->die = sheep_die;
        self->touch = sheep_touch;
	self->takedamage = DAMAGE_YES;
        self->svflags |= SVF_MONSTER;
        self->classname = "misc_sheep";

        gi.linkentity (self);

        self->count = (int)random();
        self->think = sheep_think;
        self->nextthink = level.time + FRAMETIME;

        M_droptofloor(self);
}
