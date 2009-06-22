/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Emeriss
SD%Complete: 90
SDComment: Teleport function & Mark of Nature missing
SDCategory: Bosses
EndScriptData */

#include "../sc_defines.h"

#define SPELL_SLEEP                24777
#define SPELL_NOXIOUSBREATH        24818
#define SPELL_TAILSWEEP            15847
//#define SPELL_MARKOFNATURE         25040   // Not working
#define SPELL_VOLATILEINFECTION    24928
#define SPELL_CORRUPTIONOFEARTH    24910