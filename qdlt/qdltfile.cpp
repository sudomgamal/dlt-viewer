/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdlt.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QFile>
#include <QtDebug>

#include "qdltfile.h"

extern "C"
{
#include "dlt_common.h"
}

QDltFile::QDltFile()
{
    filterFlag = false;
    sortByTimeFlag = false;
    sortByTimestampFlag = false;

    cache.setMaxCost(1000);
    cacheEnable = true;
}

QDltFile::~QDltFile()
{
    clear();
}

void QDltFile::setCacheSize(qsizetype cost)
{
    if(cost==0)
    {
        cacheEnable = false;
        cache.setMaxCost(1);
    }
    else
    {
        cacheEnable = true;
        cache.setMaxCost(cost);
    }
}

void QDltFile::setDLTv2Support(bool _dltv2Support)
{
    dltv2Support = _dltv2Support;
}

bool QDltFile::getDLTv2Support() const
{
    return dltv2Support;
}

void QDltFile::clear()
{
    for(int num=0;num<files.size();num++)
    {
        if(files[num]->infile.isOpen()) {
             files[num]->infile.close();
        }
        delete(files[num]);
    }
    files.clear();

    cache.clear();
}

int QDltFile::getNumberOfFiles() const
{
    return files.size();
}

void QDltFile::setDltIndex(QVector<qint64> &_indexAll, int num)
{
    if(num<0 || num>=files.size())
    {
        return;
    }

    files[num]->indexAll = _indexAll;
}

int QDltFile::size() const
{
    int size=0;
    for(int num=0;num<files.size();num++)
    {
      if (nullptr!=files[num])
         size += files[num]->indexAll.size();
    }

    return size;
}

qint64 QDltFile::fileSize() const
{
    qint64 size=0;

    for(int num=0;num<files.size();num++)
    {
      if (nullptr!=files[num])
         size += files[num]->infile.size();
    }

    return size;
}

int QDltFile::sizeFilter() const
{
    if(filterFlag)
        return indexFilter.size();
    else
        return size();
}

bool QDltFile::open(QString _filename, bool append)
{
    qDebug() << "Open file" << _filename << "started";

    /* check if file is already opened */
    if(!append)
        clear();

    /* create new file item */
    QDltFileItem *item = new QDltFileItem();
    files.append(item);

    /* set new filename */
    item->infile.setFileName(_filename);

    /* open the log file read only */
    if(item->infile.open(QIODevice::ReadOnly)==false)
    {
        /* open file failed */
        qWarning() << "open of file" << _filename << "failed";
        return false;
    }

    return true;
}

void QDltFile::clearIndex()
{
    for(int num=0;num<files.size();num++)
    {
        files[num]->indexAll.clear();
    }
}

bool QDltFile::createIndex()
{
    bool ret = false;


    clearIndex();

    /* Example to use QtConcurrent to call updateIndex in separate Thread*/
    //QFuture<bool> f = QtConcurrent::run(this, &QDltFile::updateIndex);
    //f.waitForFinished();

    ret = updateIndex();

    //qDebug() << "Create index finished - " << size() << "messages found";

    return ret;
}

bool QDltFile::updateIndex()
{
    QByteArray buf;
    qint64 pos = 0;
    quint16 last_message_length = 0;
    quint8 version=1;
    qint64 lengthOffset=2;
    qint64 storageLength=0;

    mutexQDlt.lock();

    for(int numFile=0;numFile<files.size();numFile++)
    {
        /* check if file is already opened */
        if(false == files[numFile]->infile.isOpen())
        {
            qDebug() << "updateMsg: Infile is not open" << files[numFile]->infile.fileName() << __FILE__ << "line" << __LINE__;
            mutexQDlt.unlock();
            return false;
        }


        /* start at last found position */
        if(files[numFile]->indexAll.size())
        {
            /* move behind last found position */
            const QVector<qint64>* const_indexAll = &(files[numFile]->indexAll);
            pos = (*const_indexAll)[files[numFile]->indexAll.size()-1];

            // first move to beginnng of last found message
            files[numFile]->infile.seek(pos);

            // read and get file storage length
            buf = files[numFile]->infile.read(14);
            if(((unsigned char)buf.at(3))==2)
            {
                storageLength = 14 + ((unsigned char)buf.at(13));
            }
            else
            {
                storageLength = 16;
            }

            // read and  get last message length
            files[numFile]->infile.seek(pos + storageLength);
            buf = files[numFile]->infile.read(7);
            version = (((unsigned char)buf.at(0))&0xe0)>>5;
            if(version==2)
            {
                lengthOffset = 5;
            }
            else
            {
                lengthOffset = 2;  // default
            }
            last_message_length = (unsigned char)buf.at(lengthOffset); // was 0
            last_message_length = (last_message_length<<8 | ((unsigned char)buf.at(lengthOffset+1))) + storageLength; // was 1

            // move just behind the next expected message
            pos += (last_message_length - 1);
            files[numFile]->infile.seek(pos);
        }
        else {
            /* the file was empty the last call */
            files[numFile]->infile.seek(0);
        }

        /* Align kbytes, 1MB read at a time */
        static const int READ_BUF_SZ = 1024 * 1024;

        /* walk through the whole file and find all DLT0x01 markers */
        /* store the found positions in the indexAll */
        char lastFound = 0;
        qint64 current_message_pos = 0;
        qint64 next_message_pos = 0;
        int counter_header = 0;
        quint16 message_length = 0;
        qint64 file_size = files[numFile]->infile.size();
        qint64 errors_in_file  = 0;

        quint8 progressNextCmdOutput=10;
        while(true)
        {
            if( (file_size>0) && ((pos*100/file_size)>=progressNextCmdOutput))
            {
                qDebug() << "CI:" << pos*100/file_size << "%";
                progressNextCmdOutput+=10;
            }

            /* read buffer from file */
            buf = files[numFile]->infile.read(READ_BUF_SZ);
            if(buf.isEmpty())
                break; // EOF

            /* Use primitive buffer for faster access */
            int cbuf_sz = buf.size();
            const char *cbuf = buf.constData();

            /* find marker in buffer */
            for(int num=0;num<cbuf_sz;num++) {
                // search length of DLT message
                if(counter_header>0)
                {
                    counter_header++;
                    if(storageLength==13 && counter_header==13)
                    {
                        storageLength += ((unsigned char)cbuf[num]) + 1;
                    }
                    else if (counter_header==storageLength)
                    {
                        // Read DLT protocol version
                        version = (((unsigned char)cbuf[num])&0xe0)>>5;
                        if(version==1)
                        {
                            lengthOffset = 2;
                        }
                        else if(version==2)
                        {
                            lengthOffset = 5;
                        }
                        else
                        {
                            lengthOffset = 2;  // default
                        }
                    }
                    else if (counter_header==(storageLength+lengthOffset)) // was 16
                    {
                        // Read low byte of message length
                        message_length = (unsigned char)cbuf[num];
                    }
                    else if (counter_header==(storageLength+1+lengthOffset)) // was 17
                    {
                        // Read high byte of message length
                        counter_header = 0;
                        message_length = (message_length<<8 | ((unsigned char)cbuf[num])) + storageLength;
                        next_message_pos = current_message_pos + message_length;
                        if(next_message_pos==file_size)
                        {
                            // last message found in file
                            files[numFile]->indexAll.append(current_message_pos);
                            break;
                        }
                        // speed up move directly to next message, if inside current buffer
                        if((message_length > storageLength+2+lengthOffset)) // was 20
                        {
                            if((num+message_length-(storageLength+2+lengthOffset)<cbuf_sz))  // was 20
                            {
                                num+=message_length-(storageLength+2+lengthOffset);  // was 20
                            }
                        }
                    }
                }
                else if(cbuf[num] == 'D')
                {
                    lastFound = 'D';
                }
                else if(lastFound == 'D' && cbuf[num] == 'L')
                {
                    lastFound = 'L';
                }
                else if(lastFound == 'L' && cbuf[num] == 'T')
                {
                    lastFound = 'T';
                }
                else if(lastFound == 'T' && (cbuf[num] == 0x01 || cbuf[num] == 0x02))
                {
                    if(next_message_pos == 0)
                    {
                        // first message detected or first message after error
                        current_message_pos = pos+num-3;
                        counter_header = 3;
                        if(cbuf[num] == 0x01)
                            storageLength = 16;
                        else
                            storageLength = 13;
                        if(current_message_pos!=0)
                        {
                            // first messages not at beginning or error occured before
                            errors_in_file++;
                        }
                        // speed up move directly to message length, if inside current buffer
                        if(num+9<cbuf_sz)
                        {
                            num+=9;
                            counter_header+=9;
                        }
                    }
                    else if( next_message_pos == (pos+num-3) )
                    {
                        // Add message only when it is in the correct position in relationship to the last message
                        files[numFile]->indexAll.append(current_message_pos);
                        current_message_pos = pos+num-3;
                        counter_header = 3;
                        if(cbuf[num] == 0x01)
                            storageLength = 16;
                        else
                            storageLength = 13;
                        // speed up move directly to message length, if inside current buffer
                        if(num+9<cbuf_sz)
                        {
                            num+=9;
                            counter_header+=9;
                        }
                    }
                    else if(next_message_pos > (pos+num-3))
                    {
                        // Header detected before end of message
                    }
                    else
                    {
                        // Header detected after end of message
                        // start search for new message back after last header found
                        files[numFile]->infile.seek(current_message_pos+4);
                        pos = current_message_pos+4;
                        buf = files[numFile]->infile.read(READ_BUF_SZ);
                        cbuf_sz = buf.size();
                        cbuf = buf.constData();
                        num=0;
                        next_message_pos = 0;
                    }
                    lastFound = 0;
                }
                else
                {
                    lastFound = 0;
                }
            }
            pos += cbuf_sz;
        }
    }

    mutexQDlt.unlock();

    /* success */
    return true;
}


bool QDltFile::createIndexFilter()
{
    /* clear old index */
    indexFilter.clear();

    return updateIndexFilter();
}

bool QDltFile::updateIndexFilter()
{
    QDltMsg msg;
    QByteArray buf;
    int index;

    /* update index filter by starting from last found index in list */

    /* get lattest found index in filter list */
    if(indexFilter.size()>0) {
        index = indexFilter[indexFilter.size()-1] + 1;
    }
    else {
        index = 0;
    }

    quint8 progressNextCmdOutput=10;
    for(int num=index;num<size();num++) {

        if( (size()>0) && ((num*100/size())>=progressNextCmdOutput))
        {
            qDebug() << "CFI:" << num*100/size() << "%";
            progressNextCmdOutput+=10;
        }

        buf = getMsg(num);
        if(!buf.isEmpty()) {
            msg.setMsg(buf,true,dltv2Support);
            msg.setIndex(num);
            if(checkFilter(msg)) {
                indexFilter.append(num);
            }
        }

    }

    return true;
}

bool QDltFile::checkFilter(QDltMsg &msg)
{
    if(!filterFlag)
    {
        return true;
    }

    return filterList.checkFilter(msg);
}

QDltFilterList QDltFile::getFilterList() const
{
    return filterList;
}

void QDltFile::setFilterList(QDltFilterList &_filterList)
{
    filterList = _filterList;
}

void QDltFile::clearFilterIndex()
{
    /* clear old index */
    indexFilter.clear();

}

void QDltFile::addFilterIndex (int index)
{
    indexFilter.append(index);

}

#ifdef USECOLOR
    QColor QDltFile::checkMarker(const QDltMsg &msg)
    {
        if(!filterFlag)
        {
            return QColor();
        }

        return filterList.checkMarker(msg);
    }

#else
 QString QDltFile::checkMarker(const QDltMsg &msg)
 {
     if(!filterFlag)
     {
         return QString(""); // invalid colour
     }

     return filterList.checkMarker(msg);
 }
#endif


QString QDltFile::getFileName(int num)
{
    if(num<0 || num>=files.size())
        return QString();

    return files[num]->infile.fileName();
}

int QDltFile::getFileMsgNumber(int num) const
{
    if(num<0 || num>=files.size())
        return -1;

    return files[num]->indexAll.size();
}

void QDltFile::close()
{
    /* close file */
    clear();
}

QByteArray QDltFile::getMsg(int index) const
{
    QByteArray buf;
    int num = 0;


    /* check if index is in range */
    if( index < 0 )
    {
        qDebug() << "getMsg: Index is out of range" << __FILE__ << "line" << __LINE__;

        /* return empty data buffer */
        return QByteArray();
    }

    for( num=0; num < files.size(); num++ )
    {
        if(index < files[num]->indexAll.size())
            break;
        else
            index -= files[num]->indexAll.size();
    }

    if(num >= files.size())
    {
     qDebug() << "getMsg: Index is out of range in" << __FILE__ << "line" << __LINE__;
     /* return empty data buffer */
     return QByteArray();
    }

    /* check if file is already opened */
    if(false == files[num]->infile.isOpen())
    {
        /* return empty buffer */
        qDebug() << "getMsg: Infile is not open" << files[num]->infile.fileName() << __FILE__ << "line" << __LINE__;

        /* return empty data buffer */
        return QByteArray();
    }

    mutexQDlt.lock();

    QDltFileItem* file = files[num];
    const QDltFileItem* const_file = file;
    qint64 positionForIndex = const_file->indexAll[index];

    /* move to file position selected by index */
    if ( false == file->infile.seek(positionForIndex) )
    {
        qDebug() << "Seek error on " << positionForIndex << file->infile.fileName() << __FILE__ << __LINE__;
        mutexQDlt.unlock();
        buf.clear();
        return buf;
    }

    /* read DLT message from file */
    if(index == (file->indexAll.size()-1))
    {
        /* last message in file */
        long int cal_index = file->infile.size() - positionForIndex;

        if ( cal_index < 0 )
            qDebug() << "Negativ index " << cal_index << index << "in" << file->infile.fileName() << __LINE__ << "of" << __FILE__;
        else
         buf = file->infile.read(file->infile.size() - positionForIndex);
    }
    else
    {
        /* any other file position */
        long int cal_index = const_file->indexAll[index+1] - positionForIndex;
        if ( cal_index < 0 )
            qDebug() << "Negativ index " << cal_index << index << "in" << __LINE__ << "of" << __FILE__;
        else
         buf = file->infile.read(cal_index);
    }

    mutexQDlt.unlock();

    /* return DLT message buffer */
    return buf;
}

bool QDltFile::getMsg(int index,QDltMsg &msg)
{
    bool result;
    QDltMsg *cacheMsg;

    // check if msg is already in cache
    if(cacheEnable)
    {
        mutexQDlt.lock();
        cacheMsg = cache[index];
        if(cacheMsg)
        {
            // load from cache
            msg = *cacheMsg;
            mutexQDlt.unlock();
            return true;
        }
        mutexQDlt.unlock();
    }

    // load message from DLT file
    QByteArray data = getMsg(index);
    if(data.isEmpty())
        return false;
    result = msg.setMsg(data,true,dltv2Support);
    msg.setIndex(index);

    // store msg in cache
    if(cacheEnable && result)
    {
        cacheMsg = new QDltMsg();
        *cacheMsg = msg;
        mutexQDlt.lock();
        if(!cache.insert(index,cacheMsg))
        {
            // object deleted already by insert function
            // delete cacheMsg;
        }
        mutexQDlt.unlock();
    }

    return result;
}

QByteArray QDltFile::getMsgFilter(int index) const
{
    if(filterFlag)
    {
        /* check if index is in range */
        if(index<0 || index>=indexFilter.size())
        {
          qDebug() << "getMsg: Index is out of range" << __FILE__ << "line" << __LINE__;
          /* return empty data buffer */
           return QByteArray();
        }
        return getMsg(indexFilter[index]);
    }
    else
    {
        /* check if index is in range */
        if(index<0 || index>=size())
        {
         qDebug() << "getMsg: Index" << index << "is out of range" << size() << __FILE__ << "line" << __LINE__;
         /* return empty data buffer */
         return QByteArray();
        }
        return getMsg(index);
    }
}

int QDltFile::getMsgFilterPos(int index) const
{
    if(filterFlag)
    {
        /* check if index is in range */
        if(index<0 || index>=indexFilter.size())
        {
        //qDebug() << "getMsg: Index is out of range" << __FILE__ << "line" << __LINE__;
        qDebug() << "getMsg: Index" << index << "is out of range" << indexFilter.size() << __FILE__ << "line" << __LINE__;
        /* return invalid */
        return -1;
        }
        return indexFilter[index];
    }
    else {
        /* check if index is in range */
        if(index<0 || index>=size())
        {
         qDebug() << "getMsg: Index is out of range" << __FILE__ << "line" << __LINE__;
        /* return invalid */
        return -1;
        }
        return index;
    }
}

void QDltFile::clearFilter()
{
    filterList.clearFilter();
}

void QDltFile::addFilter(QDltFilter *_filter)
{
    filterList.addFilter(_filter);
}

void QDltFile::updateSortedFilter()
{
    filterList.updateSortedFilter();
}

bool QDltFile::isFilter() const
{
    return filterFlag;
}

void QDltFile::enableFilter(bool state)
{
    filterFlag = state;
}

void QDltFile::enableSortByTime(bool state)
{
    sortByTimeFlag = state;
}

void QDltFile::enableSortByTimestamp(bool state)
{
    sortByTimestampFlag = state;
}

QVector<qint64> QDltFile::getIndexFilter() const
{
    return indexFilter;
}

void QDltFile::setIndexFilter(QVector<qint64> _indexFilter)
{
    indexFilter = _indexFilter;
}

bool QDltFile::applyRegExString(QDltMsg &msg,QString &text)
{

    return filterList.applyRegExString(msg,text);
}

bool QDltFile::applyRegExStringMsg(QDltMsg &msg) const
{    
    return filterList.applyRegExStringMsg(msg);
}
