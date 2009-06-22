/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: The_Barrens
SD%Complete: 5
SDComment: Quest support: 863
SDCategory: Barrens
EndScriptData */

/* ContentData
npc_beaten_corpse
npc_sputtervalve
EndContentData */

#include "../../sc_defines.h"

/*######
## npc_beaten_corpse
######*/

bool GossipHello_npc_beaten_corpse(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(4921) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(4921) == QUEST_STATUS_COMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,"Examine corpse in detail...",GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(3557, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_beaten_corpse(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF +1)
    {
        pPlayer->SEND_GOSSIP_MENU(3558, pCreature->GetGUID());
        pPlayer->KilledMonster(10668, pCreature->GetGUID());
    }
    return true;
}

/*######
## npc_sputtervalve
######*/

bool GossipHello_npc_sputtervalve(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(6981) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,"Can you tell me about this shard?",GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_sputtervalve(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->SEND_GOSSIP_MENU(2013, pCreature->GetGUID());
        //pPlayer->AreaExploredOrEventHappens(6981);
    }
    return true;
}

/*######
## npc_taskmaster_fizzule
######*/

enum
{
    FACTION_FRIENDLY_F  = 35,
    SPELL_FLARE         = 10113,
    SPELL_FOLLY         = 10137,
};

struct MANGOS_DLL_DECL npc_taskmaster_fizzuleAI : public ScriptedAI
{
    npc_taskmaster_fizzuleAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        factionNorm = pCreature->getFaction();
        Reset();
    }

    uint32 factionNorm;
    bool IsFriend;
    uint32 Reset_Timer;
    uint8 FlareCount;

    void Reset()
    {
        IsFriend = false;
        Reset_Timer = 120000;
        FlareCount = 0;
        m_creature->setFaction(factionNorm);
    }

    void DoFriend()
    {
        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);

        m_creature->StopMoving();
        //m_creature->GetMotionMaster()->MoveIdle();

        m_creature->setFaction(FACTION_FRIENDLY_F);
        m_creature->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_FLARE || spell->Id == SPELL_FOLLY)
        {
            ++FlareCount;

            if (FlareCount >= 2)
                IsFriend = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (IsFriend)
        {
            if (Reset_Timer < diff)
            {
                EnterEvadeMode();
            } else Reset_Timer -= diff;
        }

        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;
		
		Unit *victim = m_creature->getVictim();
        DoStartMeleeAttack(victim);
    }

    void ReciveEmote(Player* pPlayer, uint32 emote)
    {
        if (emote == TEXTEMOTE_SALUTE)
        {
            if (FlareCount >= 2)
            {
                if (m_creature->getFaction() == FACTION_FRIENDLY_F)
                    return;

                DoFriend();
            }
        }
    }
};

CreatureAI* GetAI_npc_taskmaster_fizzule(Creature* pCreature)
{
    return new npc_taskmaster_fizzuleAI(pCreature);
}

/*#####
## npc_twiggy_flathead
#####*/

#define SAY_BIG_WILL_READY                  "Ready when you are, $c." // SAY; LANG_UNIVERSAL; EMOTE: 15
#define SAY_TWIGGY_BEGIN                    "The Affray has begun. $n, get ready to fight!" // SAY; LANG_UNIVERSAL;
#define SAY_TWIGGY_FRAY                     "You! Enter the fray!" // SAY; LANG_UNIVERSAL;
#define SAY_TWIGGY_DOWN                     "Challenger is down!" // SAY; LANG_UNIVERSAL;
#define SAY_TWIGGY_OVER                     "The Affray is over." // SAY; LANG_UNIVERSAL;

#define NPC_TWIGGY                          6248
#define NPC_BIG_WILL                        6238
#define NPC_AFFRAY_CHALLENGER               6240
#define QUEST_AFFRAY                        1719

float AffrayChallengerLoc[6][4]=
{
    {-1683, -4326, 2.79, 0},
    {-1682, -4329, 2.79, 0},
    {-1683, -4330, 2.79, 0},
    {-1680, -4334, 2.79, 1.49},
    {-1674, -4326, 2.79, 3.49},
    {-1677, -4334, 2.79, 1.66}
};

struct MANGOS_DLL_DECL npc_twiggy_flatheadAI : public ScriptedAI
{
    npc_twiggy_flatheadAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    bool EventInProgress;

    uint32 Event_Timer;
    uint32 Step;
    uint32 Challenger_Count;
    uint32 ChallengerDeath_Timer;

    uint64 PlayerGUID;
    uint64 BigWillGUID;
    uint64 AffrayChallenger[6];

    void Reset()
    {
        EventInProgress = false;

        Event_Timer = 2000;
        Step = 0;
        Challenger_Count = 0;
        ChallengerDeath_Timer = 0;

        PlayerGUID = 0;
        BigWillGUID = 0;

        for(uint8 i = 0; i < 6; i++)
            AffrayChallenger[i] = 0;
    }

    bool CanStartEvent(Player* pPlayer)
    {
        if (!EventInProgress)
        {
            EventInProgress = true;
            PlayerGUID = pPlayer->GetGUID();
			DoSay(SAY_TWIGGY_BEGIN, LANG_UNIVERSAL, pPlayer);
            return true;
        }

        debug_log("SD2: npc_twiggy_flathead event already in progress, need to wait.");
        return false;
    }

    void SetChallengers()
    {
        for(uint8 i = 0; i < 6; i++)
        {
            Creature* pCreature = m_creature->SummonCreature(NPC_AFFRAY_CHALLENGER, m_creature->GetMapId(), AffrayChallengerLoc[i][0], AffrayChallengerLoc[i][1], AffrayChallengerLoc[i][2], AffrayChallengerLoc[i][3], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
            if (!pCreature)
            {
                debug_log("SD2: npc_twiggy_flathead event cannot summon challenger as expected.");
                continue;
            }

            pCreature->setFaction(35);
            pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            pCreature->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
            AffrayChallenger[i] = pCreature->GetGUID();
        }
    }

    void SetChallengerReady(Unit *pUnit)
    {
        pUnit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        pUnit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        pUnit->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
        pUnit->setFaction(14);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!EventInProgress)
            return;

        if (ChallengerDeath_Timer)
        {
            if (ChallengerDeath_Timer < diff)
            {
                for(uint8 i = 0; i < 6; i++)
                {
                    /*Creature *challenger = (Creature*)Unit::GetUnit(*m_creature,AffrayChallenger[i]);
                    if (challenger && !challenger->isAlive() && challenger->isDead())
                    {
                        DoSay(SAY_TWIGGY_DOWN, LANG_UNIVERSAL, NULL);
                        challenger->RemoveCorpse();
                        AffrayChallenger[i] = 0;
                        continue;
                    }*/ 
                }
                ChallengerDeath_Timer = 2500;
            } else ChallengerDeath_Timer -= diff;
        }

        if (Event_Timer < diff)
        {
            /*Player* pPlayer = (Player*)Unit::GetUnit(*m_creature,PlayerGUID);

            if (!pPlayer || pPlayer->isDead())
                Reset();*/

            switch(Step)
            {
                case 0:
                    SetChallengers();
                    ChallengerDeath_Timer = 2500;
                    Event_Timer = 5000;
                    ++Step;
                    break;
                case 1:
					DoSay(SAY_TWIGGY_FRAY, LANG_UNIVERSAL, NULL);
                    /*if (Unit *challenger = Unit::GetUnit(*m_creature,AffrayChallenger[Challenger_Count]))
                        SetChallengerReady(challenger);
                    else Reset();*/
                    ++Challenger_Count;
                    Event_Timer = 25000;
                    if (Challenger_Count == 6)
                        ++Step;
                    break;
                case 2:
                    if (Unit *temp = m_creature->SummonCreature(NPC_BIG_WILL, m_creature->GetMapId(),-1713.79,-4342.09,6.05,6.15,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,300000))
                    {
                        BigWillGUID = temp->GetGUID();
                        temp->setFaction(35);
                        //temp->GetMotionMaster()->MovePoint(0,-1682.31,-4329.68,2.78);
                    }
                    Event_Timer = 15000;
                    ++Step;
                    break;
                case 3:
                    /*if (Unit *will = Unit::GetUnit(*m_creature,BigWillGUID))
                    {
                        will->setFaction(32);
                        DoSay(SAY_BIG_WILL_READY, LANG_UNIVERSAL, pPlayer);
                    }*/
                    Event_Timer = 5000;
                    ++Step;
                    break;
                case 4:
                    /*Unit *will = Unit::GetUnit(*m_creature,BigWillGUID);
                    if (will && will->isDead())
                    {
                        DoSay(SAY_TWIGGY_OVER, LANG_UNIVERSAL, NULL);
                        Reset();
                    } else if (!will)
                        Reset();*/
                    Event_Timer = 5000;
                    break;
            }
        } else Event_Timer -= diff;
    }
};

CreatureAI* GetAI_npc_twiggy_flathead(Creature* pCreature)
{
    return new npc_twiggy_flatheadAI(pCreature);
}

bool AreaTrigger_at_twiggy_flathead(Player* pPlayer, Quest *pQuest, uint32 pAt)
{
    if (!pPlayer->isDead() && pPlayer->GetQuestStatus(QUEST_AFFRAY) == QUEST_STATUS_INCOMPLETE)
    {
        /*if (uint16 slot = pPlayer->FindQuestSlot(QUEST_AFFRAY))
        {
            //we don't want player to start event if failed already.
            if (pPlayer->GetQuestSlotState(slot) == QUEST_STATE_FAIL)
                return true;
        }*/

        /*Creature* pCreature = GetClosestCreatureWithEntry(pPlayer, NPC_TWIGGY, 30.0f);

        if (!pCreature)
            return true;

        if (((npc_twiggy_flatheadAI*)pCreature->AI())->CanStartEvent(pPlayer))
            return false;                                   //ok to let mangos process further
        else
            return true;*/
    }
    return true;
}

void AddSC_the_barrens()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_beaten_corpse";
    newscript->pGossipHello = &GossipHello_npc_beaten_corpse;
    newscript->pGossipSelect = &GossipSelect_npc_beaten_corpse;
    m_scripts[nrscripts++] = newscript;

	newscript = new Script;
    newscript->Name = "npc_sputtervalve";
    newscript->pGossipHello = &GossipHello_npc_sputtervalve;
    newscript->pGossipSelect = &GossipSelect_npc_sputtervalve;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name = "npc_taskmaster_fizzule";
    newscript->GetAI = &GetAI_npc_taskmaster_fizzule;
    m_scripts[nrscripts++] = newscript;

	newscript = new Script;
    newscript->Name = "npc_twiggy_flathead";
    newscript->GetAI = &GetAI_npc_twiggy_flathead;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name = "at_twiggy_flathead";
    newscript->pAreaTrigger = &AreaTrigger_at_twiggy_flathead;
    m_scripts[nrscripts++] = newscript;
}