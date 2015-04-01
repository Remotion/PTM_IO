#include "PTM_IO.h"
#include "PresetTools.h"
#include "re_exVector.h"

//-----------------------------------------------------------------------------
LONG ReadGrayRAW(BaseFile *file, TMap2d<UCHAR> &bmp,LONG width,LONG height, LONG format, LONG part)
{
	if(!file) return IMAGE_NOTEXISTING;

	Bool noerr = FALSE;

	LONG mult = 6;
	LONG ofR = 0;
	LONG ofG = 1;
	LONG ofB = 2;
	
	if(format==PTM_FORMAT_RGB){   //A(012345)
		mult = 6;
		// NOT SUPPORTED !!!!
	}else
	if(format==PTM_FORMAT_LUM){//A(012345) cRcB
		mult = 8; 
		// NOT SUPPORTED !!!!
	}else
	if(format==PTM_FORMAT_LRGB){ //A(012345) RGB
		if(part==0){
			mult = 3; 

			ofR = 0; //R
			ofG = 1; //G
			ofB = 2; //B
		}else if(part==1){
			mult = 6; 

			ofR = 0; //a0
			ofG = 1; //a1
			ofB = 2; //a2
		}else if(part==2){
			mult = 6; 

			ofR = 3; //a3
			ofG = 4; //a4
			ofB = 5; //a5
		}
	}else return IMAGE_WRONGTYPE;
	
	//if (bmp->Init(width,height,24)==IMAGE_OK)
	if(bmp.Init(width,height))
	{
		UINT scan_line_len;
		UCHAR *Psb = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);

		UCHAR  R = 0;
		UCHAR  G = 0;
		UCHAR  B = 0;

		for (LONG y=height-1; y>=0; y--){
			scan_line_len = file->TryReadBytes(Psb,width*mult);
			if(scan_line_len!=width*mult){ print("len err"); break; } //??????
			for(LONG x=0; x<width; x++){
				R = Psb[x*mult+ofR];
				G = Psb[x*mult+ofG];
				B = Psb[x*mult+ofB];
				bmp.SetPixel(x,y,R);
				//bmp->SetPixel(x,y,R,G,B);
			}
		}
		GeFree(Psb);
		noerr = TRUE;
	}

	if(noerr) return IMAGE_OK;
	else      return IMAGE_NOMEM;
}

//-----------------------------------------------------------------------------
LONG ReadGrayRAW(BaseFile *file, TMap2d<TryByte> &bmp,LONG width,LONG height, LONG format, LONG part)
{
	if(!file) return IMAGE_NOTEXISTING;

	Bool noerr = FALSE;

	LONG mult = 6;
	LONG ofR = 0;
	LONG ofG = 1;
	LONG ofB = 2;
	
	if(format==PTM_FORMAT_RGB){   //A(012345)
		mult = 6;
		// NOT SUPPORTED !!!!
	}else
	if(format==PTM_FORMAT_LUM){//A(012345) cRcB
		mult = 8; 
		// NOT SUPPORTED !!!!
	}else
	if(format==PTM_FORMAT_LRGB){ //A(012345) RGB
		if(part==0){
			mult = 3; 

			ofR = 0; //R
			ofG = 1; //G
			ofB = 2; //B
		}else if(part==1){
			mult = 6; 

			ofR = 0; //a0
			ofG = 1; //a1
			ofB = 2; //a2
		}else if(part==2){
			mult = 6; 

			ofR = 3; //a3
			ofG = 4; //a4
			ofB = 5; //a5
		}
	}else return IMAGE_WRONGTYPE;
	
	//if (bmp->Init(width,height,24)==IMAGE_OK)
	if(bmp.Init(width,height))
	{
		UINT scan_line_len;
		UCHAR *Psb = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);

		UCHAR  R = 0;
		UCHAR  G = 0;
		UCHAR  B = 0;
		TryByte   tb;

		for (LONG y=height-1; y>=0; y--){
			scan_line_len = file->TryReadBytes(Psb,width*mult);
			if(scan_line_len!=width*mult){ print("len err"); break; } //??????
			for(LONG x=0; x<width; x++){
				R = Psb[x*mult+ofR];
				G = Psb[x*mult+ofG];
				B = Psb[x*mult+ofB];
				tb = TryByte(R,G,B);
				bmp.SetPixel(x,y,tb);
			}
		}
		GeFree(Psb);
		noerr = TRUE;
	}

	if(noerr) return IMAGE_OK;
	else      return IMAGE_NOMEM;
}

//-----------------------------------------------------------------------------
LONG ReadA6RAW(BaseFile *file, TMap2d<TryByte> &a123, TMap2d<TryByte> &a456,LONG width,LONG height)
{
	if(!file) return IMAGE_NOTEXISTING;

	Bool noerr = FALSE;

	LONG mult = 6;

	//if (bmp->Init(width,height,24)==IMAGE_OK)
	if(a123.Init(width,height) && a456.Init(width,height))
	{
		UINT scan_line_len;
		UCHAR *Psb = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);

		TryByte a1;
		TryByte a3;

		for (LONG y=height-1; y>=0; y--){
			scan_line_len = file->TryReadBytes(Psb,width*mult);
			if(scan_line_len!=width*mult){ print("len err"); break; } //??????
			for(LONG x=0; x<width; x++){
				a1.x = Psb[x*mult+0];
				a1.y = Psb[x*mult+1];
				a1.z = Psb[x*mult+2];

				a3.x = Psb[x*mult+3];
				a3.y = Psb[x*mult+4];
				a3.z = Psb[x*mult+5];

				a123.SetPixel(x,y,a1);
				a456.SetPixel(x,y,a3);
			}
		}
		GeFree(Psb);
		noerr = TRUE;
	}

	if(noerr) return IMAGE_OK;
	else      return IMAGE_NOMEM;
}

//-----------------------------------------------------------------------------
LONG ReadRGBRAW(BaseFile *file, TMap2d<TryByte> &rgb, LONG width,LONG height)
{
	if(!file) return IMAGE_NOTEXISTING;

	Bool noerr = FALSE;

	LONG mult = 3; //RGB

	if(rgb.Init(width,height))
	{
		UINT scan_line_len;
		UCHAR *Psb = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);

		TryByte cc;

		for (LONG y=height-1; y>=0; y--){
			scan_line_len = file->TryReadBytes(Psb,width*mult);
			if(scan_line_len!=width*mult){ print("len err"); break; } //??????
			for(LONG x=0; x<width; x++){
				cc.x = Psb[x*mult+0];
				cc.y = Psb[x*mult+1];
				cc.z = Psb[x*mult+2];

				rgb.SetPixel(x,y,cc);
			}
		}
		GeFree(Psb);
		noerr = TRUE;
	}

	if(noerr) return IMAGE_OK;
	else      return IMAGE_NOMEM;
}

/*
#include "jpegdecoder.h"
#include "JPG_Stream.h"
*/
/*
//---------------------------------------------------------------------------
LONG ReadTryJPG(BaseFile *file, TMap2d<TryByte> &rgb, 
				LONG fileRpos, LONG fileGpos,LONG fileBpos)//, LONG frame)
{
	if(!file) return IMAGE_NOTEXISTING;
	TryByte cc;

	Bool noerr = TRUE;
	JPG_FileStream	*stream  = gNew JPG_FileStream(file); if(!stream){ noerr = FALSE; goto ERROR; }
	jpeg_decoder	*decoder = NULL;		

	//RED
	stream->SetFilePos(fileRpos); // !!!!!!
	decoder = gNew jpeg_decoder(stream,false);	if(!decoder){ noerr = FALSE; goto ERROR; }
	if (decoder->get_error_code() != 0) { print("PTMio Decoder Error 1"); noerr = FALSE; goto ERROR; }
	if (decoder->begin()){ noerr = FALSE; goto ERROR; }
	if (decoder->get_bytes_per_pixel()!=1){ print("PTMio Wrong Pixels"); noerr = FALSE; goto ERROR; }
	
	LONG width  = decoder->get_width();
	LONG height = decoder->get_height();

	if(rgb.Init(width,height))
	{
		void *Pscan_line_ofs;
		UINT scan_line_len;
		UCHAR *Psb = NULL;

		UWORD  R = 0;
		UWORD  G = 0;
		UWORD  B = 0;

		// RED 

		//for (LONG y=0; y<height; y++){
		for (LONG y=height-1; y>=0; y--){
			if (decoder->decode(&Pscan_line_ofs, &scan_line_len)){ break; }
			if (scan_line_len!=width){ break; } //??????
			//bmp->SetLine(y,Pscan_line_ofs,8);
			
			Psb = (UCHAR*)Pscan_line_ofs;
			for(LONG x=0; x<width; x++){
				cc.x = Psb[x];
				rgb.SetPixel(x,y,cc);
			}
		}
		// GREEN 
		gDelete(decoder); decoder = NULL;

		stream->SetFilePos(fileGpos); // !!!!!!
		decoder = gNew jpeg_decoder(stream,false);	if(!decoder){ noerr = FALSE; goto ERROR; }
		if (decoder->get_error_code() != 0) { print("PTMio Decoder Error 2"); noerr = FALSE; goto ERROR; }
		if (decoder->begin()){ noerr = FALSE; goto ERROR; }

		//for (LONG y=0; y<height; y++){
		for (LONG y=height-1; y>=0; y--){
			if (decoder->decode(&Pscan_line_ofs, &scan_line_len)){ break; }
			if (scan_line_len!=width){ break; } //??????
			//bmp->SetLine(y,Pscan_line_ofs,8);
			
			Psb = (UCHAR*)Pscan_line_ofs;
			for(LONG x=0; x<width; x++){
				cc = rgb.GetPixel(x,y);
				cc.y = Psb[x];
				rgb.SetPixel(x,y,cc);
			}
		}
		// BLUE 
		gDelete(decoder); decoder = NULL;

		stream->SetFilePos(fileBpos); // !!!!!!
		decoder = gNew jpeg_decoder(stream,false);	if(!decoder){ noerr = FALSE; goto ERROR; }
		if (decoder->get_error_code() != 0) { print("PTMio Decoder Error 3"); noerr = FALSE; goto ERROR; }
		if (decoder->begin()){ noerr = FALSE; goto ERROR; }

		//for (LONG y=0; y<height; y++){
		for (LONG y=height-1; y>=0; y--){ //???
			if (decoder->decode(&Pscan_line_ofs, &scan_line_len)){ break; }
			if (scan_line_len!=width){ break; } //??????
			//bmp->SetLine(y,Pscan_line_ofs,8);
			
			Psb = (UCHAR*)Pscan_line_ofs;
			for(LONG x=0; x<width; x++){
				cc = rgb.GetPixel(x,y);
				cc.z = Psb[x];
				rgb.SetPixel(x,y,cc);
			}
		}
ERROR:
		gDelete(stream);
		gDelete(decoder);
	}

	if(noerr) return IMAGE_OK;
	else      return IMAGE_NOMEM;
}
*/

//-----------------------------------------------------------------------------
LONG WriteLRGB(Filename name, 
				TMap2d<Vector> &t_rgb, 
				TMap2d<Vector> &t_a012, 
				TMap2d<Vector> &t_a345,
				Vector t_scale012,
				Vector t_scale345,
				Vector t_bias012, 
				Vector t_bias345
				)
{	
	Bool ok = FALSE;

	AutoAlloc<BaseFile>	file;	if (!file) return FALSE;
	if(file->Open(name,GE_WRITE,FILE_NODIALOG,GE_MOTOROLA))
	{
		String str;
		LONG width = t_rgb.GetXres();
		LONG height = t_rgb.GetYres();

		// Write Information
		WriteLineN(file,"PTM_1.2");	
		WriteLineN(file,"PTM_FORMAT_LRGB");
		//Xres
		str = LongToString(width); 
		WriteLineN(file,str);
		//Yres
		str = LongToString(height); 
		WriteLineN(file,str);
		//Scale 012345
		str = RealToString(t_scale012.x,-1,6)+" "+RealToString(t_scale012.y,-1,6)+" "+RealToString(t_scale012.z,-1,6)+" "
			 +RealToString(t_scale345.x,-1,6)+" "+RealToString(t_scale345.y,-1,6)+" "+RealToString(t_scale345.z,-1,6);
		WriteLineN(file,str);
		//Bias 012345
		str = LongToString(t_bias012.x*COLOR)+" "+LongToString(t_bias012.y*COLOR)+" "+LongToString(t_bias012.z*COLOR)+" "
			 +LongToString(t_bias345.x*COLOR)+" "+LongToString(t_bias345.y*COLOR)+" "+LongToString(t_bias345.z*COLOR);
		WriteLineN(file,str);

		LONG mult;
		//Write A012345 >>>>>>>>>>>>>>>>
		mult = 6;
		UCHAR *Paaa = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);
		
		Vector a0;
		Vector a3;

		for (LONG y=height-1; y>=0; y--){
			for(LONG x=0; x<width; x++){
				a0 = ((t_a012.GetPixel(x,y) / t_scale012) + t_bias012);
				a3 = ((t_a345.GetPixel(x,y) / t_scale345) + t_bias345);

				//if(a0.x < 0.0) print("a0 ",a0.x); else if(a0.x > 1.0) print("a0 ",a0.x);
				//if(a0.y < 0.0) print("a1 ",a0.y); else if(a0.y > 1.0) print("a1 ",a0.y);
				//if(a0.z < 0.0) print("a2 ",a0.z); else if(a0.z > 1.0) print("a2 ",a0.z);
				
				//if(a3.x < 0.0) print("a3 ",a3.x); else if(a3.x > 1.0) print("a3 ",a3.x);
				//if(a3.y < 0.0) print("a4 ",a3.y); else if(a3.y > 1.0) print("a4 ",a3.y);
				//if(a3.z < 0.0) print("a5 ",a3.z); else if(a3.z > 1.0) print("a5 ",a3.z);
				
				a0 = Cut(a0,0.0,1.0);
				a3 = Cut(a3,0.0,1.0);

				Paaa[x*mult+0] = (a0.x*COLOR);
				Paaa[x*mult+1] = (a0.y*COLOR);
				Paaa[x*mult+2] = (a0.z*COLOR);

				Paaa[x*mult+3] = (a3.x*COLOR);
				Paaa[x*mult+4] = (a3.y*COLOR);
				Paaa[x*mult+5] = (a3.z*COLOR);
			}
			file->WriteBytes(Paaa,width*mult);
		}
		GeFree(Paaa);


		//Write RGB >>>>>>>>>>>>>>>>
		mult = 3;
		UCHAR *Prgb = (UCHAR*)GeAlloc(width*mult*sizeof(UCHAR)+16);
		Vector rgb;

		for (LONG y=height-1; y>=0; y--){
			for(LONG x=0; x<width; x++){
				rgb = t_rgb.GetPixel(x,y);

				rgb = Cut(rgb,0.0,1.0);

				Prgb[x*mult+0] = (rgb.x*COLOR);
				Prgb[x*mult+1] = (rgb.y*COLOR);
				Prgb[x*mult+2] = (rgb.z*COLOR);
			}
			file->WriteBytes(Prgb,width*mult);
		}
		GeFree(Prgb);
	}

	return 0;
}





