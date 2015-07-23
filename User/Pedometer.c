//ADC采样通道
#define X_CHANNEL 0
#define Y_CHANNEL 1
#define Z_CHANNEL 2

//保存步长值
#define STEP_LENGTH_HIGH (Step_Result[3])
#define STEP_LENGTH_LOW  (Step_Result[4])

#define TIMEWINDOW_MIN 2   		//时间窗，×0.02s＝0.2s
#define TIMEWINDOW_MAX 20		//时间窗，×0.02s＝2s
#define REGULATION	4			//认为找到稳定规律所需要的步数
#define INVALID		2			//认为失去稳定规律所需要的步数

#define TWI_ADDRESS 0x88		//TWI/I2C设备地址

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
unsigned char Step_Result[11] = {0,0,0,1,0,0,0,0,0,0,0}; //前3个用于存放需要发送给controller的3 Bytes步数结果，后2个用于存放步长值STEP_LENGTH_HIGH和STEP_LENGTH_LOW,最后6个用于存放加速采样的结果
unsigned char Temp_Result[6] = {0,0,0,0,0,0};
unsigned long int STEPS;	
unsigned long int STEPS_Temp;	
unsigned  int _array0[3]={1,1,1};
unsigned  int _array1[3]={1,1,1};
unsigned  int _array2[3]={0,0,0};
unsigned  int _array3[3]={0,0,0};

unsigned char Interval=0;		//记录时间间隔数
unsigned char TempSteps=0;		//记步缓存
unsigned char InvalidSteps=0;	//无效步缓存
unsigned char ReReg=2;			//记录是否重新开始寻找规律
										//	2-新开始
										//	1-已经开始，但是还没有找到规律
										//	0-已经找到规律

/*------------------------------------------------------------------------------------------------------------------------
*Name: 		TimeWindow()
*Function:	实现"时间窗"算法,认为只有在有效"时间窗"内的记步才有效,而且起始时需要连续出现有效步才认为开始，20ms执行一次
*Input:		void
*Output: 	void
*------------------------------------------------------------------------------------------------------------------------*/
void TimeWindow()
{
	if(ReReg==2)		//如果是新开始的第一步，直接在记步缓存中加1
	{
		TempSteps++;
		Interval=0;		//100ms
		ReReg=1;
		InvalidSteps=0;	
	}
	else				//如果不是新开始的第一步
	{
		if((Interval>=TIMEWINDOW_MIN)&&(Interval<=TIMEWINDOW_MAX))	//如果时间间隔在有效的时间窗内
		{
			InvalidSteps=0;	
			if(ReReg==1)					//如果还没有找到规律
			{
				TempSteps++;				//记步缓存加1
				if(TempSteps>=REGULATION)	//如果记步缓存达到所要求的规律数
				{
					ReReg=0;				//已经找到规律
					STEPS=STEPS+TempSteps;	//更新显示
					TempSteps=0;
				}
				Interval=0;
			}
			else if(ReReg==0)				//如果已经找到规律，直接更新显示
			{
				STEPS++;
				TempSteps=0;
				Interval=0;
			}
		}
		else if(Interval<TIMEWINDOW_MIN)	//如果时间间隔小于时间窗下限
		{	
			if(ReReg==0)					//如果已经找到规律
			{
				if(InvalidSteps<255) 	InvalidSteps++;	//无效步缓存加1
				if(InvalidSteps>=INVALID)				//如果无效步达到所要求的数值，则重新寻找规律
				{	
					InvalidSteps=0;
					ReReg=1;
					TempSteps=1;
					Interval=0;
				}
				else									//否则，只丢弃这一次的记步，但是继续记步，不需要重新寻找规律
				{
					Interval=0;
				}
			}
			else if(ReReg==1)				//如果还没有找到规律，则之前的寻找规律过程无效，重新寻找规律
			{
				InvalidSteps=0;	
				ReReg=1;
				TempSteps=1;
				Interval=0;
			}
		}
		else if(Interval>TIMEWINDOW_MAX)	//如果时间间隔大于时间窗上限，记步已经间断，重新寻找规律
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
*Function:	实现Pedometer的基本算法，20ms执行一次。
*Input:		void
*Output: 	void
*------------------------------------------------------------------------------------------------------------------------*/
void step_counter()
{
		//----------------------------------------------ADC采样----------------------//
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
	
		//----------------------------------计算动态门限和动态精度-----------------------//
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
		
		//--------------------------线性移位寄存器--------------------------------------

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

		//------------------------- 动态门限判决 ----------------------------------
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

		//---------------------------------- 保存结果-----------------------------------------
		Save_Result();
}