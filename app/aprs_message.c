/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 standalone messaging draft/editor. No hardware TX or persistent writes.
 */
#include "app/aprs_format.h"
#include "aprs_message.h"
#ifdef ENABLE_APRS_SESSION
#include "aprs_session.h"
#endif
#include <string.h>
#ifdef ENABLE_APRS_TRANSPORT
#include "aprs_runtime.h"
static char quickEdit[APRS_MESSAGE_MAX+1];
void MESSAGE_EditQuick(unsigned slot)
{
    if(slot>=APRS_QUICK_COUNT) return;
    MESSAGE_Init();strcpy(quickEdit,gAPRS_Quick[slot]);
    gAPRS_Message.selected=slot;gAPRS_Message.page=MESSAGE_QUICK_EDIT;
    gAPRS_Message.error=MESSAGE_OK;gAPRS_Message.tapTicks=0;
}
const char *MESSAGE_QuickEditText(void) {return quickEdit;}
#endif
APRS_Message gAPRS_Message;
char gAPRS_Quick[APRS_QUICK_COUNT][APRS_MESSAGE_MAX+1];
void MESSAGE_Init(void)
{
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    static bool ready;
    if(ready) return;
    static const char defaults[]="ARRIVED SAFELY\0NEED ASSISTANCE\0CAN YOU COPY?";
    const char *p=defaults;
    for(unsigned i=0;i<APRS_QUICK_COUNT;i++) {strcpy(gAPRS_Quick[i],p);p+=strlen(p)+1;}
    ready=true;
#endif
}
static const char keyText[]=" 0\0.,?!/+-1\0ABC2\0DEF3\0GHI4\0JKL5\0MNO6\0PQRS7\0TUV8\0WXYZ9";
static const uint8_t keyOffsets[10]={0,3,12,17,22,27,32,37,43,48};
static bool Alnum(char c) { return (c>='A' && c<='Z') || (c>='0' && c<='9'); }
bool MESSAGE_ValidCall(const char *s)
{
    if (!s) return false;
    unsigned n=0;
    while (Alnum(s[n]) && n<6) ++n;
    if (!n) return false;
    if (!s[n]) return true;
    if (s[n++]!='-') return false;
    if (s[n]<'0' || s[n]>'9') return false;
    unsigned ssid=s[n++]-'0';
    if (s[n]>='0' && s[n]<='9') {
        if (ssid!=1) return false;
        ssid=ssid*10+s[n++]-'0';
    }
    return ssid<=15 && !s[n];
}
size_t MESSAGE_CallLength(const char *s)
{
    size_t n=strlen(s);
    return n>=2 && s[n-2]=='-' && s[n-1]=='0' ? n-2 : n;
}
bool MESSAGE_CallsMatch(const char *a, const char *b)
{
    size_t n=MESSAGE_CallLength(a);
    return n==MESSAGE_CallLength(b) && !memcmp(a,b,n);
}
static bool ValidId(const char *s)
{
    if (!s || !*s) return false;
    unsigned n=0;
    while (n<5 && Alnum(s[n])) ++n;
    return !s[n];
}
static void Commit(void) { gAPRS_Message.tapTicks=0; }
void MESSAGE_EditCall(void)
{
#ifdef ENABLE_APRS_SESSION
    strcpy(gAPRS_Message.editCall,gAPRS_Profile.callsign);
#endif
    gAPRS_Message.page=MESSAGE_CALLSIGN;
    gAPRS_Message.error=MESSAGE_OK;Commit();
}
void MESSAGE_Open(const char *replyTo)
{
    MESSAGE_Init();
    /* Keep the existing draft when re-entering; never discard text on PTT. */
    if (replyTo && MESSAGE_ValidCall(replyTo)) {
        strcpy(gAPRS_Message.recipient,replyTo);
        gAPRS_Message.page=MESSAGE_COMPOSE;
    } else gAPRS_Message.page=MESSAGE_RECIPIENT;
    Commit();
}
void MESSAGE_Tick500ms(void)
{
    if (gAPRS_Message.tapTicks) --gAPRS_Message.tapTicks;
}
void MESSAGE_Key(KEY_Code_t key)
{
    APRS_Message *m=&gAPRS_Message;
    m->error=MESSAGE_OK;
    if (key==KEY_EXIT) {
        Commit();
#ifndef ENABLE_APRS_CALLSIGN_ONLY
        switch (m->page) {
        case MESSAGE_OPTIONS: case MESSAGE_QUICK: case MESSAGE_REVIEW:
            m->page=MESSAGE_COMPOSE; break;
        case MESSAGE_COMPOSE: m->page=MESSAGE_RECIPIENT; break;
        default: m->page=MESSAGE_CLOSED; break;
        }
#else
        m->page=MESSAGE_CLOSED;
#endif
        return;
    }
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    if (m->page==MESSAGE_OPTIONS || m->page==MESSAGE_QUICK) {
        const unsigned count=m->page==MESSAGE_OPTIONS ? 3 : APRS_QUICK_COUNT;
        if (key==KEY_UP) m->selected=(m->selected+count-1)%count;
        else if (key==KEY_DOWN) m->selected=(m->selected+1)%count;
        else if (key==KEY_MENU) {
            if (m->page==MESSAGE_QUICK) {
                /* Insertion preserves the draft and refuses partial/truncated text. */
                unsigned n=strlen(m->text), add=strlen(gAPRS_Quick[m->selected]);
                if (!add) { m->error=MESSAGE_EMPTY; return; }
                if (n+add+(n!=0)<=APRS_MESSAGE_MAX) {
                    if (n) m->text[n++]=' ';
                    strcpy(m->text+n,gAPRS_Quick[m->selected]);
                    m->page=MESSAGE_COMPOSE;
                } else m->error=MESSAGE_FULL;
            } else if (m->selected==0) { m->selected=0; m->page=MESSAGE_QUICK; }
            else if (m->selected==1) {
                if (MESSAGE_ValidCall(m->recipient) && m->text[0]) m->page=MESSAGE_REVIEW;
                else m->error=m->text[0] ? MESSAGE_BAD_CALL : MESSAGE_EMPTY;
            } else m->page=MESSAGE_RECIPIENT;
        }
        return;
    }
    if (m->page==MESSAGE_REVIEW) {
        if (key==KEY_MENU) {
            #ifdef ENABLE_APRS_TRANSPORT
            if(RUNTIME_SendDraft()) m->page=MESSAGE_STATUS;
            else m->error=MESSAGE_TX_BLOCKED;
#else
            /* Receiver/editor-only build has no transport. */
            m->delivery=MESSAGE_TX_UNAVAILABLE; m->page=MESSAGE_STATUS;
#endif
        }
        return;
    }
    #ifdef ENABLE_APRS_TRANSPORT
    if(m->page==MESSAGE_STATUS && key==KEY_MENU) RUNTIME_Cancel();
#endif
    if (m->page==MESSAGE_STATUS || m->page==MESSAGE_CLOSED) return;
    const bool call=m->page==MESSAGE_RECIPIENT || m->page==MESSAGE_CALLSIGN;
    char *text=m->page==MESSAGE_CALLSIGN ? m->editCall : call ? m->recipient : m->text;
    #ifdef ENABLE_APRS_TRANSPORT
    if(m->page==MESSAGE_QUICK_EDIT) text=quickEdit;
#endif
    const unsigned limit=call ? 9 : APRS_MESSAGE_MAX;
#else
    if(m->page!=MESSAGE_CALLSIGN) return;
    char *text=m->editCall;
    const unsigned limit=9;
#endif
    unsigned n=strlen(text);
    if (key==KEY_MENU) {
        Commit();
        if (m->page==MESSAGE_CALLSIGN) {
#ifdef ENABLE_APRS_SESSION
            if (SESSION_SetCall(text)) m->page=MESSAGE_CLOSED;
            else m->error=MESSAGE_BAD_CALL;
#endif
        }
#ifndef ENABLE_APRS_CALLSIGN_ONLY
        #ifdef ENABLE_APRS_TRANSPORT
        else if(m->page==MESSAGE_QUICK_EDIT) {
            if(SESSION_SetQuick(m->selected,text)) m->page=MESSAGE_CLOSED;
        }
#endif
        else if (m->page==MESSAGE_RECIPIENT) {
            if (MESSAGE_ValidCall(text)) { text[MESSAGE_CallLength(text)]=0; m->page=MESSAGE_COMPOSE; }
            else m->error=MESSAGE_BAD_CALL;
        } else { m->page=MESSAGE_OPTIONS; m->selected=0; }
#endif
        return;
    }
    if (key==KEY_F) { Commit(); return; }
    if (key==KEY_STAR) {
        Commit(); if (n) text[n-1]=0; m->delivery=MESSAGE_DRAFT; return;
    }
    if (key>KEY_9) return;
    const char *group=keyText+keyOffsets[key];
    if (m->tapTicks && m->tapKey==key && n) {
        m->tapIndex=(m->tapIndex+1)%strlen(group);
    } else {
        if (n>=limit) { m->error=MESSAGE_FULL; return; }
        text[n++]=' ';text[n]=0;m->tapIndex=0;
    }
    text[n-1]=group[m->tapIndex];
    m->tapKey=key; m->tapTicks=2; m->delivery=MESSAGE_DRAFT;
}
static void Address(uint8_t *out, const char *call, bool last)
{
    unsigned i=0, ssid=0;
    while (call[i] && call[i]!='-') { out[i]=call[i]<<1; ++i; }
    unsigned n=i;
    if (call[i]=='-') { ++i; while (call[i]) ssid=ssid*10+call[i++]-'0'; }
    while(n<6) out[n++]=' '<<1;
    out[6]=0x60|(ssid<<1)|last;
}
static void MessageHeader(uint8_t *out, const char *sender, const char *recipient)
{
    Address(out,"APZUVK",false); Address(out+7,sender,true);
    out[14]=3;out[15]=0xf0;out[16]=':';
    memset(out+17,' ',9);memcpy(out+17,recipient,strlen(recipient));out[26]=':';
}
size_t MESSAGE_BuildFrame(const char *sender, const char *recipient, const char *text,
                         const char *id, uint8_t *out, size_t capacity)
{
    if (!MESSAGE_ValidCall(sender) || !MESSAGE_ValidCall(recipient) || !text ||
        !ValidId(id) || !out) return 0;
    unsigned n=0;
    while(text[n] && n<=APRS_MESSAGE_MAX) {
        if (text[n]<' ' || text[n]>'~' || text[n]=='{' || text[n]=='|' || text[n]=='~') return 0;
        ++n;
    }
    const unsigned size=16+11+n+1+strlen(id);
    if (!n || n>APRS_MESSAGE_MAX || size>capacity) return 0;
    MessageHeader(out,sender,recipient);
    memcpy(out+27,text,n);out[27+n]='{';memcpy(out+28+n,id,strlen(id));
    return size;
}

size_t MESSAGE_BuildBeacon(const char *sender, int32_t lat, int32_t lon,
                           char table, char symbol, uint8_t *out, size_t capacity)
{
    if(!MESSAGE_ValidCall(sender) || lat < -90000000 || lat > 90000000 ||
       lon < -180000000 || lon > 180000000 || !out || capacity<36 ||
       (table!='/' && table!='\\') || symbol<'!' || symbol>'~') return 0;
    Address(out,"APZUVK",false);Address(out+7,sender,true);
    out[14]=3;out[15]=0xf0;out[16]='!';
    unsigned n=17;n+=APRS_FormatDM((char *)out+n,lat,false,false);out[n++]=table;
    n+=APRS_FormatDM((char *)out+n,lon,true,false);out[n++]=symbol;
    return n;
}
