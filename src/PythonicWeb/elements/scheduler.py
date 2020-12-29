import sys, logging, pickle, datetime, os, signal, time, itertools, tty, termios, select
from element_types import Record, Function

    
class Element(Function):

    def __init__(self, config, inputData, returnPipe):
        super().__init__(config, inputData, returnPipe)


    def execute(self):

        #interval_str, interval_index, offset, log_state = self.config
        x = self.config

        mode = ''
        interval = 0
        timebase = ''
        dayOfWeek = {}
        recordDone = Record(True, None, None)


        for attrs in self.config['SpecificConfig']:
            if attrs['Name'] == 'Mode':
                mode = attrs['Data']
            elif attrs['Name'] == 'Timebase':
                timebase = attrs['Data']
            elif attrs['Name'] == 'Intervalinput':
                interval = attrs['Data']
            elif attrs['Name'] == 'StartTime':
                startTime = attrs['Data']
            elif attrs['Name'] == 'EndTime':
                endTime = attrs['Data']
            elif attrs['Name'] == 'SpecificTime':
                specTime = attrs['Data']
            elif attrs['Name'] == 'Monday':
                dayOfWeek['Monday'] = attrs['Data']
            elif attrs['Name'] == 'Tuesday':
                dayOfWeek['Tuesday'] = attrs['Data']
            elif attrs['Name'] == 'Wednesday':
                dayOfWeek['Wednesday'] = attrs['Data']
            elif attrs['Name'] == 'Thursday':
                dayOfWeek['Thursday'] = attrs['Data']
            elif attrs['Name'] == 'Friday':
                dayOfWeek['Friday'] = attrs['Data']
            elif attrs['Name'] == 'Saturday':
                dayOfWeek['Saturday'] = attrs['Data']
            elif attrs['Name'] == 'Sunday':
                dayOfWeek['Sunday'] = attrs['Data']

        # Setup interval

        if timebase == 'Seconds':
            interval = int(interval)
        elif timebase == 'Minutes':
            interval = int(interval) * 60
        elif timebase == 'Hours':
            interval == int(interval) * 3600

        # Switch modes

        if mode == "None":
            recordDone = Record(True, "Data", "LogMessage")
        elif mode == "Interval":
            cnt = 0
            while True:
                time.sleep(interval)

                if self.bStop:
                    recordDone = Record(False, cnt, None, True) # Exit message
                    # Necessary to end the ProcessHandler     
                    self.returnPipe.send(recordDone)
                    break      


                recordDone = Record(False, cnt, None)     
                self.returnPipe.send(recordDone)
                cnt += 1

        elif mode == "Interval between times":
            recordDone = Record(True, "Data", "LogMessage")
        elif mode == "At specific time":
            recordDone = Record(True, "Data", "LogMessage")
        elif mode == "On every full interval":
            recordDone = Record(True, "Data", "LogMessage")
        elif mode == "Full interval between times":
            recordDone = Record(True, "Data", "LogMessage")
        """
        n_cnt = 5
        while n_cnt > 0:
            time.sleep(1)
            n_cnt -= 1
            intemediateRecord = Record(False, "DataIntermediate", "Log")
            
            self.returnPipe.send(intemediateRecord)
            logging.debug("Scheduler Called - {}".format(n_cnt))
        """

        
        
        #recordDone = Record(True, "Data", "LogMessage")
        #self.returnPipe.send(recordDone)