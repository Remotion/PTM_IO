CONTAINER XPTMshader
{
	NAME XPTMshader;
	INCLUDE Xbase;

	GROUP ID_SHADERPROPERTIES
	{
		FILENAME XPTM_FILE_NAME {}
		LONG XPTM_PART 
		{
			CYCLE{
				PRT_0;
				PRT_1;		
				PRT_2;	
				PRT_3;
				PRT_4;
					
				//PRT_5;
				//PRT_6;	
				//PRT_7;	
				//PRT_8;		
				//PRT_9;															
			}
		}
		
		SEPARATOR { LINE; }
		
		VECTOR XPTM_SCALE_012		{}
		VECTOR XPTM_SCALE_345		{}	
		
		SEPARATOR { LINE; }
		
		VECTOR XPTM_BIAS_012		{}	
		VECTOR XPTM_BIAS_345		{}	
		
		SEPARATOR { LINE; }
		
	}
}
