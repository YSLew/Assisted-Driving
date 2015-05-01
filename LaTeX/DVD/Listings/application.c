	hc_sr04_send_command(START_STOP, 1);	//start condition (enable the extern vhdl components, begin to trigger sensors)
	delay_ms(5);							//wait until next i2c transmission
	hc_sr04_send_command(TRIGGER, 1);		//set alternative trigger mode for ultrasonic sensors
	delay_ms(5);

	for(i=0; i<ARRAY_MAX; i++){

		#ifdef DEVICE_0
		/*****************device0**********************/
		temp = hc_sr04_read_distance(DEVICE0, TEMP);				 //read distance value of device
		if ((temp >= MIN_RANGE) && (temp <= MAX_RANGE) && (i > 3)) { //range check and average filter
			j = 0;
			sum = 0;
			if(converted_array0[i-4] != 0) {						 //only filter values which aren't zero (valid)
				sum += converted_array0[i-4];
				j++;
			}
			if(converted_array0[i-3] != 0) {
				sum += converted_array0[i-3];
				j++;
			}
			if(converted_array0[i-2] != 0) {
				sum += converted_array0[i-2];
				j++;
			}
			if(converted_array0[i-1] != 0) {
				sum += converted_array0[i-1];
				j++;
			}
			j++;													//increment j to consider current read distance value
			converted_array0[i] = (sum + temp) / j;
		}
		else {
			converted_array0[i] = 0;								//value out of range
		}
		delay_ms(40);	//read i2c distance data every 40ms (equals trigger frequency of each sensor)
		#endif