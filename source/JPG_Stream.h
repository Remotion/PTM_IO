#ifndef _JPG_STREAM_H_
#define _JPG_STREAM_H_

//File Stream TODO:
class JPG_FileStream : public jpeg_decoder_stream
{
private:
	BaseFile *file;
	LONG	 startPos;
	LONG	 thisPos;
	LONG	 endPos;
public:
	JPG_FileStream(BaseFile *bf, LONG startP=-1, LONG endP=-1)  
	{ 
		file = bf; startPos = startP; endPos = endP; thisPos = 0; 
		if(file) thisPos = file->GetPosition();
	}
	~JPG_FileStream(void)  { }
	void SetFilePos(LONG pos) { if(pos>=0) thisPos = pos; }

	virtual INT read(UCHAR *Pbuf, INT max_bytes_to_read, bool *Peof_flag);
	virtual void attach(void);
	virtual void detach(void);
};
inline void JPG_FileStream::attach(void)
{
}

inline void JPG_FileStream::detach(void)
{
}
//-----------------------------------------------------------
inline INT JPG_FileStream::read(UCHAR *Pbuf, INT max_bytes_to_read, bool *Peof_flag)
{
	if(file==NULL || Pbuf==NULL) return -1;

	//if(startPos>=0) file->Seek(startPos,GE_START);
	//print((LONG)max_bytes_to_read);

	file->Seek(thisPos,GE_START);
	INT read_len = file->TryReadBytes(Pbuf,max_bytes_to_read);
	thisPos = file->GetPosition();

	if(read_len==max_bytes_to_read) return read_len;
	else { (*Peof_flag) = true;  return read_len; }
}
//-----------------------------------------------------------




#endif