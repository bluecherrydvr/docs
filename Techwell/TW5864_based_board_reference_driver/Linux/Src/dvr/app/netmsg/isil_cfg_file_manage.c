
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <fcntl.h>


#include "isil_cfg_file_manage.h"
#include "isil_debug.h"
#include "config.h"


static S32 IsSpace(S32 c)
{
	if (c == ' ' || c == '\t')
	{
		return ISIL_TRUE;
	}
	return ISIL_FALSE;
}

static S32 IsEOL(S32 c)
{
	if (!c || c == '\n' || c == '\r')
		return ISIL_TRUE;
	return ISIL_FALSE;
}

/********************************************************
 *Function: ISIL_GetFileSize
 *Description: 获得文件大小
 *Input:
 *	S08 *sPath:文件的绝对路径
 *output:
 *       No
 *return:
 *        Success, 文件大小; Fail, -1
 *******************************************************/

S32 ISIL_GetFileSize(S08 *sPath)
{
	S32 iRet;
	struct stat buf;

	iRet = stat(sPath, &buf);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Get file state error", __FUNCTION__);
		return ISIL_FAIL;
	}
	ISIL_TRACE(BUG_NORMAL, "[%s] file size %d", __FUNCTION__, buf.st_size);

	return buf.st_size;
}


/********************************************************
 *Function: ISIL_FileToLine
 *Description: 将文件指针移动到指定行上
 *Input:
 *	FILE * fp:文件fd
 *	S32 line:指定的行号
 *output:
 *       No
 *return:
 *        OK, 1; FALAE, 0
 *******************************************************/

S32 ISIL_FileToLine(FILE * fp, S32 line)
{
	S08 buf[LINE_BUF_LEN];
	S32 i;
	rewind(fp);
	for (i = 1; i < line; i++) {
		if (fgets(buf, LINE_BUF_LEN, fp) == NULL)
			return ISIL_FALSE;
	}
	return ISIL_TRUE;
}

/********************************************************
 *Function: ISIL_GetField
 *Description: 获得指定行的各个域值
 *Input:
 *	S08 *line,行的内容指针
 *	S08 *spS08, 包含的字符串
 *	S32 f,所要找的域值位置
 *output:
 *       No
 *return:
 *        Sucess,返回域值指针; Fail, NULL
 *******************************************************/

S08 * ISIL_GetField(S08 *line, S08 *spS08, S32 f)
{
	static S08 field[LINE_BUF_LEN];
	S32 i = 0;
	S32 iSize;
	S08 spc[] = " \t\n\r", *sp = spc, *p1 = NULL, *p2 = NULL;
    iSize = (S32)sizeof(field) - 1;
	if (spS08 && *spS08)
		sp = spS08;

	if (f == -1) {		/* the last feild */
		p2 = &line[strlen(line)] - 1;
		while (strchr(sp, *p2) != 0 && p2 >= line)
			p2--;	/* the last sep */
		p1 = p2;
		while (strchr(sp, *p1) == 0 && p1 >= line)
			p1--;
		p2++;
		p1++;
		i = -1;
	} else {
		for (; i < f && *line; i++) {
			while (*line && strchr(sp, *line) != 0)
				line++;	/* the head sep */
			p1 = line;

			while (*line && strchr(sp, *line) == 0)
				line++;
			p2 = line;
			if (*line)
				line++;
		}
	}
	if (i != f || (p2 - p1) <= 0)
		return NULL;	/* not enought feild */
	strncpy(field, p1, min(p2 - p1, iSize));
	field[min(p2 - p1, iSize)] = '\0';

	return field;
}

/********************************************************
 *Function: ISIL_GetLine
 *Description: 从指定文件中获得当前行
 *Input:
 *	FILE * fp,文件指针
 *output:
 *       No
 *return:
 *        Sucess,返回行内容指针; Fail, NULL
 *******************************************************/

S08 * ISIL_GetLine(FILE * fp)
{
	static S08 line[LINE_BUF_LEN];
	S32 i;
	S08 *p;

	while (fgets(line, sizeof (line), fp) != NULL) {
		if (IsEOL(*line))
			continue;
		p = line;
		while (!IsEOL(*p))
			p++;
		/* delete last space */
		while (p >= line && (IsSpace(*p) || IsEOL(*p)))
			p--;
		*(p + 1) = 0;

		/* delete first space */
		p = line;
		while (*p && IsSpace(*p))
			p++;
		/* move */
		i = 0;
		while (*p)
			line[i++] = *p++;
		line[i] = 0;
		if (*line == 0)
			continue;
		return line;
	}
	return NULL;
}

static S32 FileExist(S08 *fname)
{
	struct stat buf;
	return stat(fname, &buf) ? ISIL_FALSE : ISIL_TRUE;
}

/********************************************************
 *Function: ISIL_FileCP
 *Description: 文件复制
 *Input:
 *	S08 *src,原文件 名
 *	S08 *dst,目标文件名
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/

S32 ISIL_FileCP(S08 *src, S08 *dst)
{
	S32 sfd, dfd, ret = ISIL_TRUE;
	S32 len;
	S08 buf[LINE_BUF_LEN];

	if (FileExist(src) == ISIL_FALSE) {
		sfd =
		    open(src, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
			 S_IRWXU | S_IRWXG | S_IRWXO);
		close(sfd);
		dfd =
		    open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
			 S_IRWXU | S_IRWXG | S_IRWXO);
		close(dfd);
		return ISIL_TRUE;
	}
	if ((sfd = open(src, O_CREAT | O_RDONLY,0644)) < 0) {
		return ISIL_FALSE;
	}

	if ((dfd =
	     open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
		  S_IRWXU | S_IRWXG | S_IRWXO)) < 0) {
		close(sfd);
		return ISIL_FALSE;
	}

	while (1) {
		len = read(sfd, buf, LINE_BUF_LEN);
		if (len == 0)
			break;
		else if (len < 0) {
			ret = ISIL_FALSE;
			break;
		}
		if (write(dfd, buf, len) != len) {
			ret = ISIL_FALSE;
			break;
		}
	}

	close(sfd);
	close(dfd);
	return ret;
}

/********************************************************
 *Function: ISIL_InsertLine
 *Description: 在文件中插入一行
 *Input:
 *	S08 *fname,文件名
 *	S08 *line, 插入行内容
 *	S32 ln,插入行号
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/

S32 ISIL_InsertLine(S08 *fname, S08 *line, S32 ln)
{
	FILE *fp, *tmp;
	S08 tmpname[PATH_MAX];
	S08 buf[LINE_BUF_LEN];

	sprintf(tmpname, "%s%s", fname, "tmp");
	if (ISIL_FileCP(fname, tmpname) != ISIL_TRUE)
		return ISIL_FALSE;
	if ((tmp = fopen(tmpname, "r+")) == NULL) {
		return ISIL_FALSE;
	}
	unlink(tmpname);
	ISIL_FileToLine(tmp, ln);
	truncate(fname, ftell(tmp));

	if ((fp = fopen(fname, "a")) == NULL) {
		fclose(tmp);
		return ISIL_FALSE;
	}
	fseek(fp, 0, 2);	/* to last place */
	fprintf(fp, "%s\n", line);
	while (fgets(buf, sizeof (buf), tmp) != NULL)
		fprintf(fp, "%s", buf);
	fclose(fp);
	fclose(tmp);
	return ISIL_TRUE;
}

/********************************************************
 *Function: ISIL_AppendLine
 *Description: 在文件中追加行
 *Input:
 *	S08 *fname,文件名
 *	S08 *line,追加行内容指针
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/

S32 ISIL_AppendLine(S08 *fname, S08 *line)
{
	FILE *fp;
	S32 c;

	if (!FileExist(fname))
		fp = fopen(fname, "w+");
	else
		fp = fopen(fname, "r+");
	if (fp == NULL)
		return ISIL_FALSE;
	fseek(fp, -1, 2);	/* to last place */
	c = fgetc(fp);
	if (!IsEOL(c))
		fprintf(fp, "\n");
	fseek(fp, 0, 2);
	fprintf(fp, "%s\n", line);
	fclose(fp);
	return ISIL_TRUE;
}

/********************************************************
 *Function: ISIL_ReplaceLine
 *Description: 用当前内容替换文件中的某行
 *Input:
 *	S08 *fname,文件名
 *	S08 *line, 替换行内容
 *	S32 ln,替换行号
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/


S32 ISIL_ReplaceLine(S08 *fname, S08 *line, S32 ln)
{
	FILE *fp, *tmp;
	S08 tmpname[PATH_MAX];
	S08 buf[LINE_BUF_LEN];
	sprintf(tmpname, "%s%s", fname, "tmp");
	if (ISIL_FileCP(fname, tmpname) != ISIL_TRUE)
		return ISIL_FALSE;
	if ((tmp = fopen(tmpname, "r+")) == NULL)
		return ISIL_FALSE;
	unlink(tmpname);
	ISIL_FileToLine(tmp, ln);
	truncate(fname, ftell(tmp));	/* delete -1 ??????? */

	if ((fp = fopen(fname, "a")) == NULL) {
		fclose(tmp);
		return ISIL_FALSE;
	}
	fseek(fp, 0, 2);	/* to last place */
	fgets(buf, LINE_BUF_LEN, tmp);	/* to skip curent line */

	fprintf(fp, "%s\n", line);
	while (fgets(buf, LINE_BUF_LEN, tmp) != NULL)
		fprintf(fp, "%s", buf);
	fclose(fp);
	fclose(tmp);
	return ISIL_TRUE;
}


/********************************************************
 *Function: ISIL_ItemIsInFileField
 *Description: 检查文件行中是否存在指定域
 *Input:
 *	S08 *fname,文件名
 *	S08 *str,域值
 *	S32 f,域在行中位置
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/

S32 ISIL_ItemIsInFileField(S08 *fname, S08 *str, S32 f)
{
	S08 buf[LINE_BUF_LEN], *p, *r;
	FILE *fp;
	S32 i = 0;
	S32 place;
	S32 found = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;
	while ((p = ISIL_GetLine(fp)) != NULL) {
		if (f == -2) {	/* compare one line */
			if (strcmp(p, str) == 0)
				found = 1;
		} else {
			if ((r = ISIL_GetField(p, NULL, f)) != NULL
			    && strcmp(r, str) == 0)
				found = 1;
		}
		if (found) {
			/* which line */
			place = ftell(fp);
			rewind(fp);
			while (fgets(buf, sizeof (buf), fp) != NULL) {
				i++;
				if (ftell(fp) == place)
					break;
			}
			fclose(fp);
			return i;
		}
	}
	fclose(fp);
	return 0;
}
/********************************************************
 *Function: ISIL_vgetw
 *Description:
 *Input:
 *	S08 *line, S08 *spS08, S32 f, S08 *field, S32 len
 *output:
 *       No
 *return:
 *        Sucess, 0, Fail, -1
 *******************************************************/

S08 * ISIL_vgetw(S08 *line, S08 *spS08, S32 f, S08 *field, S32 len)
{
	S32 i = 0;
	U32 uiSize = 0;
	S08 spc[] = " \t\n\r", *sp = spc, *p1 = NULL, *p2 = NULL;

	if (spS08 && *spS08)
		sp = spS08;
	memset(field, 0x00,len);
	i = min(len - 1, p2 - p1);
	if(i > 0)
        uiSize = i;
    i = 0;
	if (f == 0) {
		while (*line && strchr(sp, *line) == NULL)
			line++;
		if (*line)
			line++;
		else
			return NULL;
		strncpy(field, line, uiSize);
		return field;
	} else if (f == -1) {	/* the last feild */
		p2 = &line[strlen(line)] - 1;
		while (strchr(sp, *p2) != NULL && p2 >= line)
			p2--;	/* the last sep */
		p1 = p2;
		while (strchr(sp, *p1) == NULL && p1 >= line)
			p1--;
		p2++;
		p1++;
		i = -1;
	} else {
		for (; i < f && *line; i++) {
			while (*line && strchr(sp, *line) != NULL)
				line++;	/* the head sep */
			p1 = line;

			while (*line && strchr(sp, *line) == NULL)
				line++;
			p2 = line;
			if (*line)
				line++;
		}
	}
	if (i != f || (p2 - p1) <= 0)
		return NULL;	/* not enought feild */
	strncpy(field, p1, uiSize);

	return field;
}

/********************************************************
 *Function: ISIL_GetCfgItem
 *Description:获得配置文件中的条目值

 *Input:
 *	S08 *sPath:存放配置文件的绝对路径

 *	ISIL_CFG_PARAM_S* pstParam:指向一个默认全局配置

 *	S32 iCfgSize:该配置文件中的条目数

 *	S32 iItemId:所要查找的条目ID

 *	S08 *buf:存放条目值的Buff

 *output:
 *       S08 *buf:存放条目值的Buff

 *return:
 *        exit : 1；not exit: 0
 *******************************************************/

S32 ISIL_GetCfgItem(S08* sPath, ISIL_CFG_PARAM_S* pstParam, S32 iCfgSize, S32 iItemId, S08 *buf)
{
	S32 iLoop = 0;
	S32 iLine = 0;
	FILE *fp;
	S08 *p;
	S08 tmp_buf[LINE_BUF_LEN];

	if(NULL == sPath)
	{
		ISIL_TRACE(BUG_ERROR, "%s path is null", __FUNCTION__);
		return ISIL_FAIL;
	}

	if(NULL == pstParam || iCfgSize <= 0)
	{
		ISIL_TRACE(BUG_ERROR, "%s ISIL_CFG_PARAM_S param error", __FUNCTION__);
		return ISIL_FAIL;
	}


	for(iLoop= 0; iLoop < iCfgSize; iLoop++)
	{
		if(pstParam[iLoop].item == iItemId)
		{
			iLine = ISIL_ItemIsInFileField(sPath, pstParam[iLoop].name, 1);
			if (iLine)
			{
				fp = fopen(sPath, "r");
				if (!fp)
				{
					return ISIL_FAIL;
				}
				ISIL_FileToLine(fp, iLine);
				p = ISIL_GetLine(fp);
				fclose(fp);
				memset(tmp_buf,0x00, sizeof (tmp_buf));
				ISIL_vgetw(p, NULL, 0, tmp_buf, sizeof(tmp_buf));
				strcpy(buf, tmp_buf);
				return ISIL_SUCCESS;
			}
			else
			{
				strcpy(buf, pstParam[iLoop].default_value);
				return ISIL_SUCCESS;
			}

		}
	}
	return ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_SetCfgItem
 *Description:设置配置文件中的条目值
 *Input:
 *	S08 *sPath:存放配置文件的绝对路径
 *	ISIL_CFG_PARAM_S* pstParam:指向一个默认全局配置
 *	S32 iCfgSize:该配置文件中的条目数
 *	S32 iItemId:所要查找的条目ID
 *	S08 *fmt:存放条目值的格式
 *output:
 *       No
 *return:
 *        exit : 1；not exit: 0
 *******************************************************/


S32 ISIL_SetCfgItem(S08* sPath, ISIL_CFG_PARAM_S* pstParam, S32 iCfgSize, S32 iItemId, S08*fmt, ...)
{
	va_list args;
	S32 iLoop;
	S32 iLine;
	S32 iRet = ISIL_SUCCESS;
	S08 vbuf[LINE_BUF_LEN];
	S08 buf[LINE_BUF_LEN];

	va_start(args, fmt);
	vsprintf(vbuf, fmt, args);
	va_end(args);

	for (iLoop = 0; iLoop < iCfgSize; iLoop++)
	{
		if (pstParam[iLoop].item == iItemId)
		{
			sprintf(buf, "%s %s", pstParam[iLoop].name, vbuf);
			iLine = ISIL_ItemIsInFileField(sPath, pstParam[iLoop].name, 1);
			if (iLine)
			{
				iRet = ISIL_ReplaceLine(sPath, buf, iLine);
				if(iRet == ISIL_FALSE)
				{
					fprintf(stderr,"Replace return err.\n");
					return ISIL_FAIL;
				}
			}
			else
			{
				iRet = ISIL_AppendLine(sPath, buf);
				if(iRet == ISIL_FALSE)
				{
					return ISIL_FAIL;
				}
			}
		}
	}

	if (iRet == ISIL_TRUE)
	{
		iRet = ISIL_SUCCESS;
	}
	return iRet;
}


/********************************************************
 *Function: ISIL_FileExit
 *Description:检查文件是否存在
 *Input:
 *	S08 *path:所要创建的文件绝对路径
 *output:
 *       No
 *return:
 *        exit : 1；not exit: 0
 *******************************************************/


S32 ISIL_FileExit(S08 *path)
{
	S32 fd;
	fd = open(path,O_RDONLY);
	if(fd == -1)
	{
		return ISIL_FALSE;
	}
	close(fd);
	return ISIL_TRUE;
}


/********************************************************
 *Function: ISIL_ReadConfigFile
 *Description:从配置文件中读数据
 *Input:
 *	S08 *path:配置文件 的文件绝对路径
 *	S32 offset:读文件的偏移量
 *	S32 size:读数据大小
 *	S32 *type:保留，目前无用
 *	void *data:数据内容
 *output:
 *       No
 *return:
 *        Success : 0; Failt: -1
 *******************************************************/

S32 ISIL_ReadConfigFile(S08 *path,S32 offset,S32 size, S32 *type,void *data)
{
	S32 fd,ret,len;
	fd = open(path,O_RDONLY|O_CREAT|O_NONBLOCK,0644);
	if(fd == -1)
	{
		ISIL_TRACE(BUG_ERROR, "%s open %s fail", __FUNCTION__, path);
		return ISIL_FAIL;
	}
	ret = lseek(fd, offset, SEEK_SET);
	if(ret == -1)
	{
		close(fd);
		ISIL_TRACE(BUG_ERROR, "%s seek %s fail", __FUNCTION__, path);
		return ISIL_FAIL;
	}
	len = 0;

	do
	{
		ret = read(fd,data,size-len);
		if(ret == 0)
			break;
		if(ret == -1)
		{
			close(fd);
			ISIL_TRACE(BUG_ERROR, "%s read %s fail", __FUNCTION__, path);
			return ISIL_FAIL;
		}
		len +=ret;
	}while(len<size);
	close(fd);
	return len;
}

/********************************************************
 *Function: ISIL_WriteConfigFile
 *Description:向配置文件中写数据
 *Input:
 *	S08 *path:配置文件 的文件绝对路径
 *	S32 offset:写文件的偏移量
 *	S32 size:写数据大小
 *	S32 *type:保留，目前无用
 *	void *data:数据内容
 *output:
 *       No
 *return:
 *        Success : 0; Failt: -1
 *******************************************************/

S32 ISIL_WriteConfigFile(S08 *path,S32 offset,S32 size, S32 *type,void *data)
{
	S32 fd,ret,len;

    if(*type == 1)/*需要覆盖原文件*/
    {
        fd = open(path,O_RDWR|O_CREAT|O_TRUNC ,0644);
    }
    else
    {
        fd = open(path,O_RDWR|O_CREAT,0644);
    }

	if(fd == -1)
	{
		ISIL_TRACE(BUG_ERROR, "%s open %s fail", __FUNCTION__, path);
		return ISIL_FAIL;
	}

	ret = lseek(fd, offset, SEEK_SET);
	if(ret == -1)
	{
		close(fd);

		ISIL_TRACE(BUG_ERROR, "%s seek %s fail", __FUNCTION__, path);
		return ISIL_FAIL;
	}
	len = 0;

	do
	{
		ret = write(fd,data,size-len);
		if(ret == -1)
		{
			close(fd);

			ISIL_TRACE(BUG_ERROR, "%s write %s fail", __FUNCTION__, path);
			return ISIL_FAIL;
		}
		len +=ret;
	}while(len<size);
	close(fd);
	return ISIL_SUCCESS;
}

/********************************************************
 *Function: ISIL_MkConfigDir
 *Description:创建配置文件目录
 *Input:
 *	No
 *output:
 *       No
 *return:
 *        Success : 0; Failt: -1
 *******************************************************/

S32 ISIL_MkConfigDir(void)
{
	DIR *dir = NULL;
	ISIL_TRACE(BUG_NORMAL, "Begin make dir\n");
    /*创建一个config根目录*/
    if(( dir = opendir(NVS_CFG_ROOT) ) == NULL)
    {
        if( mkdir(NVS_CFG_ROOT, O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] creat config ROOT dir fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
    }
	if( ( dir = opendir(NVS_CONFIG_DIR) ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat config dir\n", __FUNCTION__, __LINE__);
		if( mkdir(NVS_CONFIG_DIR, O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] creat config dir fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
		if( ( dir = opendir(NVS_CONFIG_DIR) ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open config dir\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close config dir fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( chdir(NVS_CONFIG_DIR) == -1 )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] cd config dir fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("video") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat video\n", __FUNCTION__, __LINE__);
		if( mkdir("video", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat video fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("video") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open video fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close video fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("RS485") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat RS485\n", __FUNCTION__, __LINE__);
		if( mkdir("RS485", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat RS485 fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("RS485") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open RS485 fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close RS485 fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("audio") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat audio\n", __FUNCTION__, __LINE__);
		if( mkdir("audio", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat audio fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("audio") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open audio fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close audio fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("encode") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat encode\n", __FUNCTION__, __LINE__);
		if( mkdir("encode", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat encode fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("encode") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open encode fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close encode fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("OSD") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat osd\n", __FUNCTION__, __LINE__);
		if( mkdir("OSD", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat osd fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("OSD") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] opem osd fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close osd fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("network") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat network\n", __FUNCTION__, __LINE__);
		if( mkdir("network", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat network fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("network") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open network fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{

		ISIL_TRACE(BUG_NORMAL, "[%s %d] close network fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("device") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat device\n", __FUNCTION__, __LINE__);
		if( mkdir("device", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat device fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("device") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open device fail\n", __FUNCTION__, __LINE__);
			return -1;
		}
	}
	if( closedir(dir) == -1)
	{

		ISIL_TRACE(BUG_NORMAL, "[%s %d] close device fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
	if( ( dir = opendir("alarm") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat alarm\n", __FUNCTION__, __LINE__);
		if( mkdir("alarm", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat alarm fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("alarm") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open alarm fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close alarm fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("stream") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat stream\n", __FUNCTION__, __LINE__);
		if( mkdir("stream", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat stream fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("stream") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open stream fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close stream fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir("user") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat user\n", __FUNCTION__, __LINE__);
		if( mkdir("user", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat user fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("user") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open user fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close user fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*save ptz ctrl file*/
    if( ( dir = opendir("ptz") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat ptz\n", __FUNCTION__, __LINE__);
		if( mkdir("ptz", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat ptz fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("ptz") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open ptz fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close ptz fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*save com file*/
    if( ( dir = opendir("com") ) == NULL )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat com\n", __FUNCTION__, __LINE__);
		if( mkdir("com", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat com fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("com") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open com fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close com fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    /*
    if( ( dir = opendir("chnmap") ) == NULL )
    {
		ISIL_TRACE(BUG_NORMAL, "[%s %d] creat chnmap\n", __FUNCTION__, __LINE__);
		if( mkdir("chnmap", O_RDWR) == -1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s %d] creat chnmap fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir("chnmap") ) == NULL )
		{

			ISIL_TRACE(BUG_NORMAL, "[%s %d] open chnmap fail\n", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] close chnmap fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    */
	return 0;
}

/*创建Local net sdk配置文件目录*/
S08 * gNetSDKCfgType[] =
{
    "devcfg",
    "",
};

S32 ISIL_NetSDKCfgDir(void)
{
	DIR *dir = NULL;
	ISIL_TRACE(BUG_NORMAL, "Begin make net sdk cfg dir\n");
    /*创建配置文件的根目录*/
    /*首先cd到根目录 */
	if( chdir("/") == -1 )
	{
		ISIL_TRACE(BUG_NORMAL, "[%s %d] cd root fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( ( dir = opendir(ISIL_LOCAL_NET_SDK_CFG_PATH) ) == NULL )
	{
		if( mkdir(ISIL_LOCAL_NET_SDK_CFG_PATH, O_RDWR) == -1)
		{
            ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
		if( ( dir = opendir(ISIL_LOCAL_NET_SDK_CFG_PATH) ) == NULL )
		{

            ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{

        ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	if( chdir(ISIL_LOCAL_NET_SDK_CFG_PATH) == -1 )
	{

        ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*创建设备配置文件的目录*/
	if( ( dir = opendir(gNetSDKCfgType[NET_SDK_DEV_CFG]) ) == NULL )
	{
		if( mkdir(gNetSDKCfgType[NET_SDK_DEV_CFG], O_RDWR) == -1)
		{

            ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
			return ISIL_FAIL;
		}

		if( ( dir = opendir(gNetSDKCfgType[NET_SDK_DEV_CFG]) ) == NULL )
		{
            ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);

			return ISIL_FAIL;
		}
	}
	if( closedir(dir) == -1)
	{

        ISIL_TRACE(BUG_NORMAL, "[%s %d]", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	return 0;
}

/*修改配置文件中某个配置数据*/
void ISIL_ModifyOSSysCfg(S08 *name,S08 *key,S08 *value)
{
    FILE *fin,*fout;
    S08 temp[LINE_BUF_LEN],*ptr,**file;
    S32 i=0,j;

    fin=fopen(name,"r");
    while(fgets(temp,LINE_BUF_LEN-1,fin))
        i++;

    rewind(fin);
    if(!(file=(char**)malloc(sizeof(char*)*i)))
        exit(1);
    for(j=0;j<i;j++)
        if(!(file[j]=(char*)malloc(sizeof(char)*LINE_BUF_LEN)))
            exit(1);
    j=0;
    while(j<i)
    {
        fgets(temp,LINE_BUF_LEN,fin);
        if((ptr=strstr(temp,key))){
            *(ptr+strlen(key)+1)=0;
            strcat(temp,value);
            strcat(temp,"\n");
        }
        strcpy(file[j++],temp);
    }
    fclose(fin);

    fout=fopen(name,"w");
    j=0;
    while(j<i)
        fputs(file[j++],fout);
    fclose(fout);

    j=0;
    while(j<i)
        free(file[j++]);
}

