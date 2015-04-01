CONTAINER XPTMgen
{
	NAME XPTMgen;
	INCLUDE Xbase;

	GROUP ID_SHADERPROPERTIES
	{
		FILENAME XPTG_FILE_NAME {  }
		LONG XPTG_PART 
		{
			CYCLE{
				PRT_0;
				PRT_1;		
				PRT_2;	
				PRT_3;
				PRT_4;
				PRT_5;
				//PRT_6;	
				//PRT_7;	
				//PRT_8;		
				//PRT_9;															
			}
		}
		
		SEPARATOR { LINE; }
		LONG XPTG_SCALE_BIAS_MD 
		{
			CYCLE{
				SBM_0;
				SBM_1;		
				SBM_2;															
			}
		}
		SEPARATOR { LINE; }
		
		VECTOR XPTG_SCALE_012		{ MIN 0.5; MAX 5.0; STEP 0.1; }
		VECTOR XPTG_SCALE_345		{ MIN 0.5; MAX 5.0; STEP 0.1; }	
		
		SEPARATOR { LINE; }
		
		VECTOR XPTG_BIAS_012		{ MIN 0.0; MAX 5.0; STEP 0.01; }	
		VECTOR XPTG_BIAS_345		{ MIN 0.0; MAX 5.0; STEP 0.01; }	
		
		SEPARATOR { LINE; }
		
		BUTTON XPTG_RECALCULATE {}
	}
}
