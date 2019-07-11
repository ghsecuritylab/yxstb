#ifndef __APP_REMINDERLIST_BIT__
#define	__APP_REMINDERLIST_BIT__

#define VOD_REMINDER_SIZE 128

typedef struct tag_vod_reminder
{
	char 	ReminderId[64];
	char  ContentID[12];
	char	ContentType[12];
	int   ReminderType;
	char  ReminderTime[16];
	char  ReminderExpireTime[16];
	char  ExtendDescription[2048];
}VOD_REMINDER;

typedef struct _REMINDER_LIST_
{
	int count;
	int size;
	VOD_REMINDER**	array;
	VOD_REMINDER*	_array[VOD_REMINDER_SIZE];
	
}REMINDER_LIST;

#ifdef __cplusplus
extern "C" {
#endif


void httpReminderRequest(int arg);
void reminder_list_sync(void);

#ifdef __cplusplus
}
#endif



#endif
