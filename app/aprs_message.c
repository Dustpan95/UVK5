/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 standalone messaging draft/editor. No hardware TX or persistent writes.
 */
#include "aprs_message.h"
#ifdef ENABLE_APRS_MESSAGING
#include "aprs_session.h"
#endif
#include <string.h>
APRS_Message gAPRS_Message;
char gAPRS_Quick[APRS_QUICK_COUNT][APRS_MESSAGE_MAX+1] = {
    "ARRIVED SAFELY", "NEED ASSISTANCE", "CAN YOU COPY?"
};
static const char *const keys[10] = {
    " 0", ".,?!/+-1", "ABC2", "DEF3", "GHI4", "JKL5",
    "MNO6", "PQRS7", "TUV8", "WXYZ9"
};
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
static size_t CallLength(const char *s)
{
    size_t n=strlen(s);
    return n>=2 && s[n-2]=='-' && s[n-1]=='0' ? n-2 : n;
}
static bool CallEqual(const char *a, const char *b)
{
    size_t n=CallLength(a);
    return n==CallLength(b) && !memcmp(a,b,n);
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
#ifdef ENABLE_APRS_MESSAGING
    strcpy(gAPRS_Message.editCall,gAPRS_Profile.callsign);
#endif
    gAPRS_Message.page=MESSAGE_CALLSIGN;
    gAPRS_Message.error=MESSAGE_OK;Commit();
}
void MESSAGE_Open(const char *replyTo)
{
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
        switch (m->page) {
        case MESSAGE_OPTIONS: case MESSAGE_QUICK: case MESSAGE_REVIEW:
            m->page=MESSAGE_COMPOSE; break;
        case MESSAGE_COMPOSE: m->page=MESSAGE_RECIPIENT; break;
        default: m->page=MESSAGE_CLOSED; break;
        }
        return;
    }
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
            /* No operational transport exists yet: never report this as sent. */
            m->delivery=MESSAGE_TX_UNAVAILABLE; m->page=MESSAGE_STATUS;
        }
        return;
    }
    if (m->page==MESSAGE_STATUS || m->page==MESSAGE_CLOSED) return;
    const bool call=m->page==MESSAGE_RECIPIENT || m->page==MESSAGE_CALLSIGN;
    char *text=m->page==MESSAGE_CALLSIGN ? m->editCall : call ? m->recipient : m->text;
    const unsigned limit=call ? 9 : APRS_MESSAGE_MAX;
    unsigned n=strlen(text);
    if (key==KEY_MENU) {
        Commit();
        if (m->page==MESSAGE_CALLSIGN) {
#ifdef ENABLE_APRS_MESSAGING
            if (SESSION_SetCall(text)) m->page=MESSAGE_CLOSED;
            else m->error=MESSAGE_BAD_CALL;
#endif
        } else if (m->page==MESSAGE_RECIPIENT) {
            if (MESSAGE_ValidCall(text)) { text[CallLength(text)]=0; m->page=MESSAGE_COMPOSE; }
            else m->error=MESSAGE_BAD_CALL;
        } else { m->page=MESSAGE_OPTIONS; m->selected=0; }
        return;
    }
    if (key==KEY_F) { Commit(); return; }
    if (key==KEY_STAR) {
        Commit(); if (n) text[n-1]=0; m->delivery=MESSAGE_DRAFT; return;
    }
    if (key>KEY_9) return;
    const char *group=keys[key];
    if (m->tapTicks && m->tapKey==key && n) {
        m->tapIndex=(m->tapIndex+1)%strlen(group);
    } else {
        if (n>=limit) { m->error=MESSAGE_FULL; return; }
        text[n++]=' ';text[n]=0;m->tapIndex=0;
    }
    text[n-1]=group[m->tapIndex];
    m->tapKey=key; m->tapTicks=2; m->delivery=MESSAGE_DRAFT;
}
bool MESSAGE_MarkSent(const char *sender, const char *id)
{
    if (!MESSAGE_ValidCall(sender) || !MESSAGE_ValidCall(gAPRS_Message.recipient) ||
        !ValidId(id) || !gAPRS_Message.text[0]) return false;
    strcpy(gAPRS_Message.sender,sender); strcpy(gAPRS_Message.id,id);
    gAPRS_Message.delivery=MESSAGE_WAIT_ACK; gAPRS_Message.page=MESSAGE_STATUS;
    return true;
}
bool MESSAGE_Acknowledge(const char *source, const char *addressee, const char *id, bool rejected)
{
    APRS_Message *m=&gAPRS_Message;
    if (!source || !addressee || !id || m->delivery!=MESSAGE_WAIT_ACK ||
        !CallEqual(source,m->recipient) || !CallEqual(addressee,m->sender) || strcmp(id,m->id)) return false;
    m->delivery=rejected ? MESSAGE_REJECTED : MESSAGE_ACKED;
    return true;
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
    Address(out,"APZUVK",false); Address(out+7,sender,true);
    out[14]=3;out[15]=0xf0;out[16]=':';
    memset(out+17,' ',9);memcpy(out+17,recipient,strlen(recipient));out[26]=':';
    memcpy(out+27,text,n);out[27+n]='{';memcpy(out+28+n,id,strlen(id));
    return size;
}

static unsigned Position(uint8_t *out, int32_t value, bool longitude)
{
    const uint32_t absolute=value<0 ? (uint32_t)-value : (uint32_t)value;
    /* Round to the nearest hundredth minute, including degree carry. Bounds
     * are checked before negation; all arithmetic fits in 32 bits. */
    const uint32_t hundredths=(absolute*3u+250u)/500u;
    const unsigned degrees=hundredths/6000u, minutes=hundredths%6000u;
    unsigned n=0;
    if(longitude) out[n++]='0'+degrees/100u;
    out[n++]='0'+degrees/10u%10u;out[n++]='0'+degrees%10u;
    out[n++]='0'+minutes/1000u;out[n++]='0'+minutes/100u%10u;
    out[n++]='.';out[n++]='0'+minutes/10u%10u;out[n++]='0'+minutes%10u;
    out[n++]=longitude ? (value<0 ? 'W' : 'E') : (value<0 ? 'S' : 'N');
    return n;
}
size_t MESSAGE_BuildBeacon(const char *sender, int32_t lat, int32_t lon,
                           char table, char symbol, uint8_t *out, size_t capacity)
{
    if(!MESSAGE_ValidCall(sender) || lat < -90000000 || lat > 90000000 ||
       lon < -180000000 || lon > 180000000 || !out || capacity<36 ||
       (table!='/' && table!='\\') || symbol<'!' || symbol>'~') return 0;
    Address(out,"APZUVK",false);Address(out+7,sender,true);
    out[14]=3;out[15]=0xf0;out[16]='!';
    unsigned n=17;n+=Position(out+n,lat,false);out[n++]=table;
    n+=Position(out+n,lon,true);out[n++]=symbol;
    return n;
}
