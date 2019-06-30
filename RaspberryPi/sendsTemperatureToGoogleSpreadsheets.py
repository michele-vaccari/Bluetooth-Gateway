import gspread
from oauth2client.service_account import ServiceAccountCredentials
import time
import sys

temperature = sys.argv[1]

scope = ['https://spreadsheets.google.com/feeds','https://www.googleapis.com/auth/drive']

credentials = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', scope)

client = gspread.authorize(credentials)

sheet = client.open("SensorData").worksheets()

for ii in sheet:
  currentTime = time.localtime()
  timeToString = time.strftime("%m/%d/%Y %H:%M:%S", currentTime)
  ii.append_row([timeToString,float(temperature)])
  print('sent the following data: ', timeToString, float(temperature))