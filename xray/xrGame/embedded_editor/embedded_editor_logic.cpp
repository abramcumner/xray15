#include "pch_script.h"
#include "embedded_editor_logic.h"
#include "../GameObject.h"
#include "../script_binder_object_wrapper.h"
#include "embedded_editor_helper.h"
#include <addons/imguinodegrapheditor/imguinodegrapheditor.h>
#include <imgui.h>

bool show_logic_editor = false;
extern CObject* object;

float r_float(CInifile::Sect* sect, LPCSTR name, float def)
{
    LPCSTR temp;
    float val = sect->line_exist(name, &temp) ? atof(temp) : def;
    return val;
}

LPCSTR r_string(CInifile::Sect* sect, LPCSTR name, LPCSTR def)
{
    LPCSTR temp;
    LPCSTR val = sect->line_exist(name, &temp) ? temp : def;
    return val;
}

bool r_bool(CInifile::Sect* sect, LPCSTR name, bool def)
{
    LPCSTR temp;
    if (!sect->line_exist(name, &temp))
        return def;
    char B[8];
    strncpy(B, temp, 7);
    B[7] = 0;
    strlwr(B);
    return xr_strcmp(B, "on") == 0 || xr_strcmp(B, "yes") == 0 || xr_strcmp(B, "true") == 0 || xr_strcmp(B, "1") == 0;
}

void fill(LPSTR dest, LPCSTR src)
{
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strcpy(dest, src);
}

enum MyNodeTypes {
    MNT_LOGIC_NODE = 0,
    MNT_WALKER_NODE,
    MNT_MOB_TRADE_NODE,
    MNT_MOB_TRADER_NODE,
    MNT_SMART_COVER_NODE,
    MNT_DEATH_NODE,
    MNT_REMARK_NODE,
    MNT_MEET_NODE,
    MNT_WOUNDED_NODE,

    MNT_UNKNOWN_NODE,
    MNT_COUNT
};

static const char* MyNodeTypeNames[MNT_COUNT]
    = { "logic", "walker", "mob_trade", "mob_trader", "smart_cover", "death", "remark", "meet", "wounded",

          "unknown" };

class XrayNode : public ImGui::Node
{
public:
    virtual void initProps(CInifile::Sect* sect) = 0;
    int getInputSlot(LPCSTR slotName)
    {
        for (int i = 0; i != IMGUINODE_MAX_INPUT_SLOTS; i++) {
            if (strncmp(InputNames[i], slotName, IMGUINODE_MAX_SLOT_NAME_LENGTH) == 0)
                return i;
        }
        return -1;
    }
    int getOutputSlot(LPCSTR slotName)
    {
        for (int i = 0; i != IMGUINODE_MAX_OUTPUT_SLOTS; i++) {
            if (strncmp(OutputNames[i], slotName, IMGUINODE_MAX_SLOT_NAME_LENGTH) == 0)
                return i;
        }
        return -1;
    }
};

class LogicNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef LogicNode ThisClass;
    LogicNode()
        : Base()
    {
    }
    static const int TYPE = MNT_LOGIC_NODE;

    float sympathy = 0.0f;
    char trade[100];
    int level_spot = -1;

    const char* getTooltip() const override { return "LogicNode tooltip."; }
    const char* getInfo() const override
    {
        return "LogicNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("LogicNode", pos, "", "active;on_trade;on_death", TYPE);
        node->fields.addField(&node->sympathy, 1, "sympathy", nullptr, 3, -1000.0f, 1000.0f);
        node->fields.addFieldTextEdit(node->trade, 100, "trade");
        node->fields.addFieldEnum(&node->level_spot, "trader\0mechanic\0guider\0quest_npc\0\0", "level_spot");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        sympathy = r_float(sect, "sympathy", 0.0f);
        fill(trade, r_string(sect, "trade", ""));
        LPCSTR temp = r_string(sect, "level_spot", "");
        if (!temp)
            level_spot = -1;
        else if (strcmp(temp, "trader") == 0)
            level_spot = 0;
        else if (strcmp(temp, "mechanic") == 0)
            level_spot = 1;
        else if (strcmp(temp, "guider") == 0)
            level_spot = 2;
        else if (strcmp(temp, "quest_npc") == 0)
            level_spot = 3;
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class MobTradeNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef MobTradeNode ThisClass;
    MobTradeNode()
        : Base()
    {
    }
    static const int TYPE = MNT_MOB_TRADE_NODE;

    const char* getTooltip() const override { return "MobTradeNode tooltip."; }
    const char* getInfo() const override
    {
        return "MobTradeNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("MobTradeNode", pos, "i", "on_info;on_info2;on_info3", TYPE);
        return node;
    }

    void initProps(CInifile::Sect* sect) override {}

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class MobTraderNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef MobTraderNode ThisClass;
    MobTraderNode()
        : Base()
    {
    }
    static const int TYPE = MNT_MOB_TRADER_NODE;

    char anim_global[100];
    bool can_talk = false;
    char tip_text[100];
    char sound_phrase[100];
    char anim_head[100];

    const char* getTooltip() const override { return "MobTraderNode tooltip."; }
    const char* getInfo() const override
    {
        return "MobTraderNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("MobTraderNode", pos, "i",
            "on_signal;on_actor_dist_ge_nvis;on_info;on_timer;on_info2;on_actor_dist_ge_nvis2;on_actor_dist_le_nvis",
            TYPE);
        node->fields.addFieldTextEdit(node->anim_global, 100, "anim_global");
        node->fields.addField(&node->can_talk, "can_talk");
        node->fields.addFieldTextEdit(node->tip_text, 100, "tip_text");
        node->fields.addFieldTextEdit(node->sound_phrase, 100, "sound_phrase");
        node->fields.addFieldTextEdit(node->anim_head, 100, "anim_head");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        fill(anim_global, r_string(sect, "anim_global", ""));
        can_talk = r_bool(sect, "can_talk", true);
        fill(tip_text, r_string(sect, "tip_text", "character_use"));
        fill(sound_phrase, r_string(sect, "sound_phrase", ""));
        fill(anim_head, r_string(sect, "anim_head", ""));
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class WalkerNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef WalkerNode ThisClass;
    WalkerNode()
        : Base()
    {
    }
    static const int TYPE = MNT_WALKER_NODE;

    char path_walk[100];
    char path_look[100];
    char team[100];
    char sound_idle[100];
    char def_state_standing[100];
    char def_state_moving1[100];
    char def_state_moving[100];
    bool can_talk = false;
    char anim_head[100];

    const char* getTooltip() const override { return "WalkerNode tooltip."; }
    const char* getInfo() const override
    {
        return "WalkerNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("WalkerNode", pos, "i",
            "on_signal;on_actor_dist_ge_nvis;on_info;on_timer;on_info2;on_actor_dist_ge_nvis2;on_actor_dist_le_nvis;on_"
            "actor_dist_le;wounded;meet",
            TYPE);
        node->fields.addFieldTextEdit(node->path_walk, 100, "path_walk");
        node->fields.addFieldTextEdit(node->path_look, 100, "path_look");
        node->fields.addFieldTextEdit(node->team, 100, "team");
        node->fields.addFieldTextEdit(node->sound_idle, 100, "sound_idle");
        node->fields.addFieldTextEdit(node->def_state_standing, 100, "def_state_standing");
        node->fields.addFieldTextEdit(node->def_state_moving1, 100, "def_state_moving1");
        node->fields.addFieldTextEdit(node->def_state_moving, 100, "def_state_moving");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        fill(path_walk, r_string(sect, "path_walk", ""));
        fill(path_look, r_string(sect, "path_look", ""));
        fill(team, r_string(sect, "team", ""));
        fill(sound_idle, r_string(sect, "sound_idle", ""));
        fill(def_state_standing, r_string(sect, "def_state_standing", ""));
        fill(def_state_moving1, r_string(sect, "def_state_moving1", ""));
        fill(def_state_moving, r_string(sect, "def_state_moving", ""));
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class SmartCoverNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef SmartCoverNode ThisClass;
    SmartCoverNode()
        : Base()
    {
    }
    static const int TYPE = MNT_SMART_COVER_NODE;

    char cover_name[100];
    char loophole_name[100];
    int cover_state;
    char target_enemy[100];
    char target_path[100];
    float idle_min_time;
    float idle_max_time;
    float lookout_min_time;
    float lookout_max_time;
    char exit_body_state[100];
    bool use_precalc_cover;
    bool use_in_combat;
    char def_state_moving[100];
    char sound_idle[100];

    const char* getTooltip() const override { return "SmartCoverNode tooltip."; }
    const char* getInfo() const override
    {
        return "SmartCoverNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("SmartCoverNode", pos, "i",
            "on_signal;on_actor_dist_ge_nvis;on_info;on_timer;on_info2;on_actor_dist_ge_nvis2;on_actor_dist_le_nvis",
            TYPE);
        node->fields.addFieldTextEdit(node->cover_name, 100, "cover_name");
        node->fields.addFieldTextEdit(node->loophole_name, 100, "loophole_name");
        node->fields.addFieldEnum(&node->cover_state,
            "nil\0default_behaviour\0idle_target\0lookout_target\0fire_target\0fire_no_lookout_target\0\0",
            "cover_state");
        node->fields.addFieldTextEdit(node->target_enemy, 100, "target_enemy");
        node->fields.addFieldTextEdit(node->target_path, 100, "target_path");
        node->fields.addField(&node->idle_min_time, 1, "idle_min_time", nullptr, 3, 0.0f, 100.0f);
        node->fields.addField(&node->idle_max_time, 1, "idle_max_time", nullptr, 3, 0.0f, 100.0f);
        node->fields.addField(&node->lookout_min_time, 1, "lookout_min_time", nullptr, 3, 0.0f, 100.0f);
        node->fields.addField(&node->lookout_max_time, 1, "lookout_max_time", nullptr, 3, 0.0f, 100.0f);
        node->fields.addFieldTextEdit(node->exit_body_state, 100, "exit_body_state");
        node->fields.addField(&node->use_precalc_cover, "use_precalc_cover");
        node->fields.addField(&node->use_in_combat, "use_in_combat");
        node->fields.addFieldTextEdit(node->def_state_moving, 100, "def_state_moving");
        node->fields.addFieldTextEdit(node->sound_idle, 100, "sound_idle");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        fill(cover_name, r_string(sect, "cover_name", "$script_id$_cover"));
        fill(loophole_name, r_string(sect, "loophole_name", ""));
        LPCSTR temp = r_string(sect, "cover_state", "");
        if (!temp)
            cover_state = 1;
        else if (strcmp(temp, "nil") == 0)
            cover_state = 0;
        else if (strcmp(temp, "default_behaviour") == 0)
            cover_state = 1;
        else if (strcmp(temp, "idle_target") == 0)
            cover_state = 2;
        else if (strcmp(temp, "lookout_target") == 0)
            cover_state = 3;
        else if (strcmp(temp, "fire_target") == 0)
            cover_state = 4;
        else if (strcmp(temp, "fire_no_lookout_target") == 0)
            cover_state = 5;
        fill(target_enemy, r_string(sect, "target_enemy", ""));
        fill(target_path, r_string(sect, "target_path", "nil"));
        idle_min_time = r_float(sect, "idle_min_time", 6.0f);
        idle_max_time = r_float(sect, "idle_max_time", 10.0f);
        lookout_min_time = r_float(sect, "lookout_min_time", 6.0f);
        lookout_max_time = r_float(sect, "lookout_max_time", 10.0f);
        fill(exit_body_state, r_string(sect, "exit_body_state", "stand"));
        use_precalc_cover = r_bool(sect, "use_precalc_cover", false);
        use_in_combat = r_bool(sect, "use_in_combat", false);
        fill(def_state_moving, r_string(sect, "def_state_moving", "sneak"));
        fill(sound_idle, r_string(sect, "sound_idle", ""));
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class DeathNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef DeathNode ThisClass;
    DeathNode()
        : Base()
    {
    }
    static const int TYPE = MNT_DEATH_NODE;

    const char* getTooltip() const override { return "DeathNode tooltip."; }
    const char* getInfo() const override
    {
        return "DeathNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("DeathNode", pos, "i", "on_info;on_info2", TYPE);
        return node;
    }

    void initProps(CInifile::Sect* sect) override {}

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class UnknownNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef UnknownNode ThisClass;
    UnknownNode()
        : Base()
    {
    }
    static const int TYPE = MNT_UNKNOWN_NODE;

    const char* getTooltip() const override { return "UnknownNode tooltip."; }
    const char* getInfo() const override
    {
        return "UnknownNode info.\n\nThis is supposed to display some info about this node.";
    }

    virtual void getDefaultTitleBarColors(
        ImU32& defaultTitleTextColorOut, ImU32& defaultTitleBgColorOut, float& defaultTitleBgColorGradientOut) const
    {
        defaultTitleTextColorOut = 0;
        defaultTitleBgColorOut = IM_COL32(125, 35, 0, 255);
        defaultTitleBgColorGradientOut = 0.025f;
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("UnknownNode", pos, "i",
            "on_signal;on_actor_dist_ge_nvis;on_info;on_timer;on_info2;on_actor_dist_ge_nvis2;on_actor_dist_le_nvis",
            TYPE);
        return node;
    }

    void initProps(CInifile::Sect* sect) override {}

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class RemarkNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef RemarkNode ThisClass;
    RemarkNode()
        : Base()
    {
    }
    static const int TYPE = MNT_REMARK_NODE;

    bool snd_anim_sync;
    char snd[100];
    char anim[100];
    char tips[100];
    char tips_sender[100];
    bool anim_reset;
    char target[100];

    const char* getTooltip() const override { return "RemarkNode tooltip."; }
    const char* getInfo() const override
    {
        return "RemarkNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("RemarkNode", pos, "i",
            "on_signal;on_actor_dist_ge_nvis;on_info;on_timer;on_info2;on_actor_dist_ge_nvis2;on_actor_dist_le_nvis",
            TYPE);

        node->fields.addField(&node->snd_anim_sync, "snd_anim_sync");
        node->fields.addFieldTextEdit(node->snd, 100, "snd");
        node->fields.addFieldTextEdit(node->anim, 100, "anim");
        node->fields.addFieldTextEdit(node->tips, 100, "tips");
        node->fields.addFieldTextEdit(node->tips_sender, 100, "tips_sender");
        node->fields.addField(&node->anim_reset, "anim_reset");
        node->fields.addFieldTextEdit(node->target, 100, "target");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        snd_anim_sync = r_bool(sect, "snd_anim_sync", false);
        fill(snd, r_string(sect, "snd", ""));
        fill(anim, r_string(sect, "anim", "wait"));
        fill(tips, r_string(sect, "tips", ""));
        fill(tips_sender, r_string(sect, "tips_sender", ""));
        anim_reset = r_bool(sect, "anim_reset", true);
        fill(target, r_string(sect, "target", "nil"));
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class MeetNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef MeetNode ThisClass;
    MeetNode()
        : Base()
    {
    }
    static const int TYPE = MNT_MEET_NODE;

    char meet_state[100];
    char meet_state_wpn[100];
    char victim[100];
    char victim_wpn[100];
    char sound_start[100];
    char sound_start_wpn[100];
    char sound_stop[100];
    char use[100];
    char use_wpn[100];
    char meet_dialog[100];
    char zone[100];
    char precond[100];
    char abuse[100];
    bool meet_only_at_path;
    bool trade_enable;
    bool allow_break;
    bool quest_npc;

    const char* getTooltip() const override { return "MeetNode tooltip."; }
    const char* getInfo() const override
    {
        return "MeetNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("MeetNode", pos, "i", "", TYPE);

        node->fields.addFieldTextEdit(node->meet_state, 100, "meet_state");
        node->fields.addFieldTextEdit(node->meet_state_wpn, 100, "meet_state_wpn");
        node->fields.addFieldTextEdit(node->victim, 100, "victim");
        node->fields.addFieldTextEdit(node->victim_wpn, 100, "victim_wpn");
        node->fields.addFieldTextEdit(node->sound_start, 100, "sound_start");
        node->fields.addFieldTextEdit(node->sound_start_wpn, 100, "sound_start_wpn");
        node->fields.addFieldTextEdit(node->sound_stop, 100, "sound_stop");
        node->fields.addFieldTextEdit(node->use, 100, "use");
        node->fields.addFieldTextEdit(node->use_wpn, 100, "use_wpn");
        node->fields.addFieldTextEdit(node->meet_dialog, 100, "meet_dialog");
        node->fields.addFieldTextEdit(node->zone, 100, "zone");
        node->fields.addFieldTextEdit(node->precond, 100, "precond");
        node->fields.addFieldTextEdit(node->abuse, 100, "abuse");
        node->fields.addField(&node->meet_only_at_path, "meet_only_at_path");
        node->fields.addField(&node->trade_enable, "trade_enable");
        node->fields.addField(&node->allow_break, "allow_break");
        node->fields.addField(&node->quest_npc, "quest_npc");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        fill(meet_state, r_string(sect, "meet_state", ""));
        fill(meet_state_wpn, r_string(sect, "meet_state_wpn", ""));
        fill(victim, r_string(sect, "victim", ""));
        fill(victim_wpn, r_string(sect, "victim_wpn", ""));
        fill(sound_start, r_string(sect, "sound_start", ""));
        fill(sound_start_wpn, r_string(sect, "sound_start_wpn", ""));
        fill(sound_stop, r_string(sect, "sound_stop", ""));
        fill(use, r_string(sect, "use", ""));
        fill(use_wpn, r_string(sect, "use_wpn", ""));
        fill(meet_dialog, r_string(sect, "meet_dialog", ""));
        fill(zone, r_string(sect, "zone", ""));
        fill(precond, r_string(sect, "precond", ""));
        fill(abuse, r_string(sect, "abuse", ""));
        meet_only_at_path = r_bool(sect, "meet_only_at_path", false);
        trade_enable = r_bool(sect, "trade_enable", true);
        allow_break = r_bool(sect, "allow_break", true);
        quest_npc = r_bool(sect, "quest_npc", false);
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

class WoundedNode : public XrayNode
{
protected:
    typedef XrayNode Base;
    typedef WoundedNode ThisClass;
    WoundedNode()
        : Base()
    {
    }
    static const int TYPE = MNT_WOUNDED_NODE;

    char hp_state[100];
    char hp_state_see[100];
    char psy_state[100];
    char hp_victim[100];
    char hp_cover[100];
    char hp_fight[100];
    char syndata[100];
    char help_dialog[100];
    char help_start_dialog[100];
    bool use_medkit;
    bool autoheal;
    bool enable_talk;

    const char* getTooltip() const override { return "WoundedNode tooltip."; }
    const char* getInfo() const override
    {
        return "WoundedNode info.\n\nThis is supposed to display some info about this node.";
    }

public:
    static ThisClass* Create(const ImVec2& pos)
    {
        ThisClass* node = (ThisClass*)ImGui::MemAlloc(sizeof(ThisClass));
        new (node) ThisClass();
        node->init("WoundedNode", pos, "i", "", TYPE);

        node->fields.addFieldTextEdit(node->hp_state, 100, "hp_state");
        node->fields.addFieldTextEdit(node->hp_state_see, 100, "hp_state_see");
        node->fields.addFieldTextEdit(node->psy_state, 100, "psy_state");
        node->fields.addFieldTextEdit(node->hp_victim, 100, "hp_victim");
        node->fields.addFieldTextEdit(node->hp_cover, 100, "hp_cover");
        node->fields.addFieldTextEdit(node->hp_fight, 100, "hp_fight");
        node->fields.addFieldTextEdit(node->syndata, 100, "syndata");
        node->fields.addFieldTextEdit(node->help_dialog, 100, "help_dialog");
        node->fields.addFieldTextEdit(node->help_start_dialog, 100, "help_start_dialog");
        node->fields.addField(&node->use_medkit, "use_medkit");
        node->fields.addField(&node->autoheal, "autoheal");
        node->fields.addField(&node->enable_talk, "enable_talk");
        return node;
    }

    void initProps(CInifile::Sect* sect) override
    {
        fill(hp_state, r_string(sect, "hp_state", ""));
        fill(hp_state_see, r_string(sect, "hp_state_see", ""));
        fill(psy_state, r_string(sect, "psy_state", ""));
        fill(hp_victim, r_string(sect, "hp_victim", ""));
        fill(hp_cover, r_string(sect, "hp_cover", ""));
        fill(hp_fight, r_string(sect, "hp_fight", ""));
        fill(syndata, r_string(sect, "syndata", ""));
        fill(help_dialog, r_string(sect, "help_dialog", ""));
        fill(help_start_dialog, r_string(sect, "help_start_dialog", ""));
        use_medkit = r_bool(sect, "use_medkit", false);
        autoheal = r_bool(sect, "autoheal", true);
        enable_talk = r_bool(sect, "enable_talk", true);
    }

    inline static ThisClass* Cast(Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
    inline static const ThisClass* Cast(const Node* n) { return Node::Cast<ThisClass>(n, TYPE); }
};

static ImGui::Node* MyNodeFactory(int nt, const ImVec2& pos, const ImGui::NodeGraphEditor& /*nge*/)
{
    switch (nt) {
    case MNT_LOGIC_NODE:
        return LogicNode::Create(pos);
    case MNT_WALKER_NODE:
        return WalkerNode::Create(pos);
    case MNT_MOB_TRADE_NODE:
        return MobTradeNode::Create(pos);
    case MNT_MOB_TRADER_NODE:
        return MobTraderNode::Create(pos);
    case MNT_SMART_COVER_NODE:
        return SmartCoverNode::Create(pos);
    case MNT_DEATH_NODE:
        return DeathNode::Create(pos);
    case MNT_REMARK_NODE:
        return RemarkNode::Create(pos);
    case MNT_MEET_NODE:
        return MeetNode::Create(pos);
    case MNT_WOUNDED_NODE:
        return WoundedNode::Create(pos);
    case MNT_UNKNOWN_NODE:
        return UnknownNode::Create(pos);
    }
    IM_ASSERT(false);
    return NULL;
}

int stringToNodeType(const xr_string& type)
{
    for (int i = 0; i < MNT_COUNT; i++)
        if (type == MyNodeTypeNames[i])
            return i;
    return MNT_COUNT - 1;
}

ImVec2 getCoor(CInifile::Sect* sect)
{
    float x = r_float(sect, "x", 0.0f);
    float y = r_float(sect, "y", 0.0f);
    return ImVec2(x, y);
}

XrayNode* getNodeByName(const ImGui::NodeGraphEditor& nge, LPCSTR name)
{
    for (int i = 0; i != nge.getNumNodes(); i++)
        if (strcmp(nge.getNode(i)->getName(), name) == 0)
            return (XrayNode*)nge.getNode(i);
    return nullptr;
}

xr_string getSection(LPCSTR expr)
{
    LPCSTR p = strpbrk(expr, "{}|%");
    if (!p)
        return expr;
    LPCSTR start = expr;
    LPCSTR div = strchr(expr, '|');
    if (div)
        start = div + 1;
    LPCSTR closeBracket = strchr(start, '}');
    if (closeBracket)
        start = closeBracket + 1;
    LPCSTR per = strchr(start, '%');
    if (per)
        return xr_string(start, per - start);
    else
        return start;
}

xr_map<xr_string, ImGui::NodeGraphEditor*> logics;

void ShowLogicEditor(bool& show)
{
    ImguiWnd wnd("Logic editor", &show);
    if (wnd.Collapsed)
        return;

    CGameObject* gameObject = smart_cast<CGameObject*>(object);
    if (!gameObject || !gameObject->object()) {
        ImGui::Text("no logic");
        return;
    }

    ImGui::Text("ID: %u Name: %s", object->ID(), object->cName().c_str());
    luabind::wrap_base* base = (CScriptBinderObjectWrapper*)gameObject->object();
    const auto& ref = luabind::detail::wrap_access::ref(*base).m_strong_ref;
    luabind::object binder(ref.state(), ref, true);
    luabind::object st = binder["st"];
    xr_string iniFile = to_string(st["ini_filename"]);
    xr_string activeSection = to_string(st["active_section"]);
    ImGui::Text("ini_filename=%s active_section=%s", iniFile.c_str(), activeSection.c_str());
    ImGui::Separator();

    string_path fileName;
    CInifile* ini = xr_new<CInifile>(FS.update_path(fileName, "$game_config$", iniFile.c_str()), TRUE, TRUE, FALSE);

    ImGui::NodeGraphEditor* nge = logics[iniFile];
    if (!nge) {
        nge = new ImGui::NodeGraphEditor();
        logics[iniFile] = nge;
    }
    if (nge->isInited()) {
        nge->registerNodeTypes(MyNodeTypeNames, MNT_COUNT, MyNodeFactory, NULL, -1);
        nge->registerNodeTypeMaxAllowedInstances(MNT_LOGIC_NODE, 1);

        int n = 0;
        for (auto el : ini->sections()) {
            LPCSTR name = el->Name.c_str();
            LPCSTR atP = strchr(name, '@');
            xr_string type = atP ? xr_string(name, atP - name) : name;
            ImVec2 coor = getCoor(el);
            if (coor.x == 0.0f && coor.y == 0.0f) {
                coor.x = 200.0f * (n % 6);
                coor.y = 200.0f * (n / 6);
            }
            ImGui::Node* node = nge->addNode(stringToNodeType(type), coor);
            nge->overrideNodeName(node, name);
            ((XrayNode*)node)->initProps(el);
            n++;
        }

        for (auto sect : ini->sections()) {
            LPCSTR name = sect->Name.c_str();
            for (const auto& prop : sect->Data) {
                LPCSTR first = prop.first.c_str();
                bool isStartOn = strstr(first, "on_") == first;
                bool isActive = strcmp(first, "active") == 0;
                bool isMeet = strcmp(first, "meet") == 0;
                bool isWounded = strcmp(first, "wounded") == 0;
                if (!isStartOn && !isActive && !isMeet && !isWounded)
                    continue;
                LPCSTR second = prop.second.c_str();
                while (true) {
                    LPCSTR colon = strchr(second, ',');
                    xr_string temp(second, colon ? colon - second : strlen(second));
                    XrayNode* src = getNodeByName(*nge, name);
                    xr_string dstName = getSection(temp.c_str());
                    XrayNode* dst = getNodeByName(*nge, dstName.c_str());
                    if (!dstName.empty() && dstName != "no_meet")
                        nge->addLink(src, src->getOutputSlot(first), dst, 0);
                    if (colon)
                        second = colon + 1;
                    else
                        break;
                }
            }
        }
        nge->show_style_editor = true;
        nge->show_load_save_buttons = true;
    }

    // highlight active section
    for (int i = 0; i != nge->getNumNodes(); i++) {
        if (nge->getNode(i)->getName() == activeSection) {
            ImU32 bg = IM_COL32(0, 128, 0, 255);
            nge->overrideNodeTitleBarColors(nge->getNode(i), nullptr, &bg, nullptr);
        } else {
            ImU32 bg = 0;
            nge->overrideNodeTitleBarColors(nge->getNode(i), nullptr, &bg, nullptr);
        }
    }
    nge->render();
}