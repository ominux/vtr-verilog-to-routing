#include <string.h>
#include <assert.h>
#include "read_xml_util.h"


static int process_settings(ezxml_t Cur, char ** outv)
{
	int count = 0;

	if(!Cur)
		return(0);

	while(Cur->attr[count])
	{
		if(outv)
		{
			if(! (count % 2))
			{
				outv[count] = (char *)my_malloc(strlen(Cur->attr[count]) + 3);
				strcpy(&outv[count][2], Cur->attr[count]);
				outv[count][0] = outv[count][1] = '-';
			}
			else
				outv[count] = Cur->attr[count];
		}
		count++;
	}

	Cur = Cur->child;

	while(Cur)
	{
		if(outv)
		{
			outv[count] = (char *)my_malloc(strlen(Cur->name) + 3);
			strcpy(&outv[count][2], Cur->name);
			outv[count][0] = outv[count][1] = '-';
		}
		count++;

		if(strlen(Cur->txt))
		{
			if(outv) outv[count] = Cur->txt;
			count++;
		}

		Cur = Cur->ordered;
	}

	return count;
}

int read_settings_file(char * file_name, char *** outv)
{
	ezxml_t Cur;
	Cur = ezxml_parse_file(file_name);
	int count;

	assert(*outv == NULL);
	assert(! strcasecmp("settings",Cur->name));
	Cur = FindElement(Cur, "arguments", 0);

	count = process_settings(Cur, *outv);

	/* prepend the settings file name */
	count++;

	*outv = (char **)my_malloc(count * sizeof(char *));

	(*outv)[0] = strdup(file_name);

	if(count)
	{
		process_settings(Cur, &((*outv)[1]));
	}

	return(count);
}
