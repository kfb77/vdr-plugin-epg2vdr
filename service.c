/*
 * service.c: EPG2VDR plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "service.h"
#include "plgconfig.h"

//***************************************************************************
// Class cEpgTimer
//***************************************************************************

cEpgTimer::cEpgTimer(bool Instant, bool Pause, const cChannel* Channel)
   : cEpgTimer_Interface_V1(Instant, Pause, Channel)
{
   timerid = na; eventid = na;
   vdrName = 0; vdrUuid = 0;
   vdrRunning = no;
   stateInfo = 0;
   local = yes;
}

cEpgTimer::~cEpgTimer()
{
   free(vdrUuid);
   free(vdrName);
   free(stateInfo);
}

void cEpgTimer::setState(char s, const char* info)
{
   state = s;
   free(stateInfo);
   stateInfo = 0;

   if (!isEmpty(info))
      stateInfo = strdup(info);
}

void cEpgTimer::setAction(char a)
{
   action = a;
}

void cEpgTimer::setVdr(const char* name, const char* uuid, int running)
{
   local = yes;   // the default
   free(vdrUuid);
   free(vdrName);
   vdrName = strdup(name);

   if (!isEmpty(uuid))
      vdrUuid = strdup(uuid);

   vdrRunning = running;

   if (!isEmpty(vdrUuid) && strcmp(vdrUuid, Epg2VdrConfig.uuid) != 0)
      local = no;
}

//***************************************************************************
// Class cEpgEvent
//***************************************************************************

cEpgEvent::cEpgEvent(tEventID EventID)
   : cEpgEvent_Interface_V1(EventID)
{

}

bool cEpgEvent::Read(FILE *f)
{
   char *s;
   int line = 0;
   cReadLine ReadLine;

   while ((s = ReadLine.Read(f)) != NULL) {
      line++;
      char *t = skipspace(s + 1);
      switch (*s) {
         case 'E': {
            unsigned int EventID;
            time_t StartTime;
            int Duration;
            unsigned int TableID = 0;
            unsigned int Version = 0xFF; // actual value is ignored
            int n = sscanf(t, "%u %ld %d %X %X", &EventID, &StartTime, &Duration, &TableID, &Version);
            if (n >= 3 && n <= 5) {
               SetTableID(TableID);
               SetStartTime(StartTime);
               SetDuration(Duration);
            }
            break;
         }
         default:  if (!Parse(s)) {
            esyslog("ERROR: EPG data problem in line %d", line);
            return false;
         }
      }
   }

   return true;
}
