//ADC����ͨ��
#define X_CHANNEL 0
#define Y_CHANNEL 1
#define Z_CHANNEL 2

//���沽��ֵ
#define STEP_LENGTH_HIGH (Step_Result[3])
#define STEP_LENGTH_LOW  (Step_Result[4])

#define TIMEWINDOW_MIN 2   		//ʱ�䴰����0.02s��0.2s
#define TIMEWINDOW_MAX 20		//ʱ�䴰����0.02s��2s
#define REGULATION	4			//��Ϊ�ҵ��ȶ���������Ҫ�Ĳ���
#define INVALID		2			//��Ϊʧȥ�ȶ���������Ҫ�Ĳ���

#define TWI_ADDRESS 0x88		//TWI/I2C�豸��ַ

unsigned char itemp,jtemp,temp;								
unsigned char _bad_flag[3];									
unsigned char Receive;	    				
unsigned char ResultIndex;										
unsigned int  Adresult;											
unsigned char  sampling_counter;								
unsigned int _adresult[3];									
unsigned int _max[3]={0,0,0};
unsigned int _min[3]={1000,1000,1000};
unsigned int _dc[3]={500,500,500};
unsigned int _vpp[3]={30,30,30};	
unsigned int  _precision[3]={5,5,5};	
unsigned int _old_fixed[3];
unsigned int _new_fixed[3];
unsigned char Readflag,save_data_flag=0,load_data_flag=0;					
unsigned char Start_flag;		
unsigned char Sample_flag;			
unsigned char Step_Result[11] = {0,0,0,1,0,0,0,0,0,0,0}; //ǰ3�����ڴ����Ҫ���͸�controller��3 Bytes�����������2�����ڴ�Ų���ֵSTEP_LENGTH_HIGH��STEP_LENGTH_LOW,���6�����ڴ�ż��ٲ����Ľ��
unsigned char Temp_Result[6] = {0,0,0,0,0,0};
unsigned long int STEPS;	
unsigned long int STEPS_Temp;	
unsigned  int _array0[3]={1,1,1};
unsigned  int _array1[3]={1,1,1};
unsigned  int _array2[3]={0,0,0};
unsigned  int _array3[3]={0,0,0};

unsigned char Interval=0;		//��¼ʱ������
unsigned char TempSteps=0;		//�ǲ�����
unsigned char InvalidSteps=0;	//��Ч������
unsigned char ReReg=2;			//��¼�Ƿ����¿�ʼѰ�ҹ���
										//	2-�¿�ʼ
										//	1-�Ѿ���ʼ�����ǻ�û���ҵ�����
										//	0-�Ѿ��ҵ�����

/*------------------------------------------------------------------------------------------------------------------------
*Name: 		TimeWindow()
*Function:	ʵ��"ʱ�䴰"�㷨,��Ϊֻ������Ч"ʱ�䴰"�ڵļǲ�����Ч,������ʼʱ��Ҫ����������Ч������Ϊ��ʼ��20msִ��һ��
*Input:		void
*Output: 	void
*------------------------------------------------------------------------------------------------------------------------*/
void TimeWindow()
{
	if(ReReg==2)		//������¿�ʼ�ĵ�һ����ֱ���ڼǲ������м�1
	{
		TempSteps++;
		Interval=0;		//100ms
		ReReg=1;
		InvalidSteps=0;	
	}
	else				//��������¿�ʼ�ĵ�һ��
	{
		if((Interval>=TIMEWINDOW_MIN)&&(Interval<=TIMEWINDOW_MAX))	//���ʱ��������Ч��ʱ�䴰��
		{
			InvalidSteps=0;	
			if(ReReg==1)					//�����û���ҵ�����
			{
				TempSteps++;				//�ǲ������1
				if(TempSteps>=REGULATION)	//����ǲ�����ﵽ��Ҫ��Ĺ�����
				{
					ReReg=0;				//�Ѿ��ҵ�����
					STEPS=STEPS+TempSteps;	//������ʾ
					TempSteps=0;
				}
				Interval=0;
			}
			else if(ReReg==0)				//����Ѿ��ҵ����ɣ�ֱ�Ӹ�����ʾ
			{
				STEPS++;
				TempSteps=0;
				Interval=0;
			}
		}
		else if(Interval<TIMEWINDOW_MIN)	//���ʱ����С��ʱ�䴰����
		{	
			if(ReReg==0)					//����Ѿ��ҵ�����
			{
				if(InvalidSteps<255) 	InvalidSteps++;	//��Ч�������1
				if(InvalidSteps>=INVALID)				//�����Ч���ﵽ��Ҫ�����ֵ��������Ѱ�ҹ���
				{	
					InvalidSteps=0;
					ReReg=1;
					TempSteps=1;
					Interval=0;
				}
				else									//����ֻ������һ�εļǲ������Ǽ����ǲ�������Ҫ����Ѱ�ҹ���
				{
					Interval=0;
				}
			}
			else if(ReReg==1)				//�����û���ҵ����ɣ���֮ǰ��Ѱ�ҹ��ɹ�����Ч������Ѱ�ҹ���
			{
				InvalidSteps=0;	
				ReReg=1;
				TempSteps=1;
				Interval=0;
			}
		}
		else if(Interval>TIMEWINDOW_MAX)	//���ʱ��������ʱ�䴰���ޣ��ǲ��Ѿ���ϣ�����Ѱ�ҹ���
		{
			InvalidSteps=0;	
			ReReg=1;						
			TempSteps=1;
			Interval=0;
		}
	}		
}


/*------------------------------------------------------------------------------------------------------------------------
*Name: 		step_counter()
*Function:	ʵ��Pedometer�Ļ����㷨��20msִ��һ�Ρ�
*Input:		void
*Output: 	void
*------------------------------------------------------------------------------------------------------------------------*/
void step_counter()
{
		//----------------------------------------------ADC����----------------------//
		for(jtemp=X_CHANNEL;jtemp<=Z_CHANNEL;jtemp++)
		{
			adc_read(jtemp);
		
			_array3[jtemp]=_array2[jtemp];
			_array2[jtemp]=_array1[jtemp];
			_array1[jtemp]=_array0[jtemp];
			
       		_array0[jtemp]=Adresult;
       		_adresult[jtemp]=_array0[jtemp]+_array1[jtemp]+_array2[jtemp]+_array3[jtemp];
      	 	_adresult[jtemp]=_adresult[jtemp]>>2;
			if (_adresult[jtemp]>_max[jtemp])               {_max[jtemp]=_adresult[jtemp];}
			if (_adresult[jtemp]<_min[jtemp])               {_min[jtemp]=_adresult[jtemp];}
		}
		
      	sampling_counter=sampling_counter+1;
	
		//----------------------------------���㶯̬���޺Ͷ�̬����-----------------------//
        if (sampling_counter==60)
        {               
          	sampling_counter=0;
				
			for(jtemp=X_CHANNEL;jtemp<=Z_CHANNEL;jtemp++)
			{
				_vpp[jtemp]=_max[jtemp]-_min[jtemp];
            	_dc[jtemp]=_min[jtemp]+(_vpp[jtemp]>>1);
				_max[jtemp]=0;
            	_min[jtemp]=1023;
				_bad_flag[jtemp]=0;

				if (_vpp[jtemp]>=160)
				{
					_precision[jtemp]=_vpp[jtemp]/32; //8
				}
            	else if ((_vpp[jtemp]>=50)&& (_vpp[jtemp]<160))            
				{
					_precision[jtemp]=5;
				}
           		else if ((_vpp[jtemp]>=15) && (_vpp[jtemp]<50))  
                {
					_precision[jtemp]=3;
				}  
			
				else
           		{ 
              		_precision[jtemp]=2;
                	_bad_flag[jtemp]=1;
            	}

			}
      	}
		
		//--------------------------������λ�Ĵ���--------------------------------------

		for(jtemp=X_CHANNEL;jtemp<=Z_CHANNEL;jtemp++)
		{
			_old_fixed[jtemp]=_new_fixed[jtemp];

        	if (_adresult[jtemp] >= _new_fixed[jtemp])                         
        	{   
         		if((_adresult[jtemp]-_new_fixed[jtemp]) >= _precision[jtemp])	
				{
					_new_fixed[jtemp]=_adresult[jtemp];
				}
        	}
        	if (_adresult[jtemp]<_new_fixed[jtemp])
       	 	{   
           		if((_new_fixed[jtemp]-_adresult[jtemp]) >= _precision[jtemp])	
				{
					_new_fixed[jtemp]=_adresult[jtemp];
				}
        	}
		}

		//------------------------- ��̬�����о� ----------------------------------
		if ((_vpp[X_CHANNEL]>=_vpp[Y_CHANNEL])&&(_vpp[X_CHANNEL]>=_vpp[Z_CHANNEL]))
		{
			if ((_old_fixed[X_CHANNEL]>=_dc[X_CHANNEL])&&(_new_fixed[X_CHANNEL]<_dc[X_CHANNEL])&&(_bad_flag[X_CHANNEL]==0))        
			{
				TimeWindow();
			} 
		}
		else if ((_vpp[Y_CHANNEL]>=_vpp[X_CHANNEL])&&(_vpp[Y_CHANNEL]>=_vpp[Z_CHANNEL]))
		{
			if ((_old_fixed[Y_CHANNEL]>=_dc[Y_CHANNEL])&&(_new_fixed[Y_CHANNEL]<_dc[Y_CHANNEL])&&(_bad_flag[Y_CHANNEL]==0))        
			{
				TimeWindow();
			}
		}
		else if ((_vpp[Z_CHANNEL]>=_vpp[Y_CHANNEL])&&(_vpp[Z_CHANNEL]>=_vpp[X_CHANNEL]))
		{
			if ((_old_fixed[Z_CHANNEL]>=_dc[Z_CHANNEL])&&(_new_fixed[Z_CHANNEL]<_dc[Z_CHANNEL])&&(_bad_flag[Z_CHANNEL]==0))        
			{
				TimeWindow();
			}
		}

		//---------------------------------- ������-----------------------------------------
		Save_Result();
}