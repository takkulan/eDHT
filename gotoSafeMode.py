import requests, sys, json, re


# Configure values below
baseUrl='https://api.particle.io/v1/devices/'
myAccessToken='key your access token here'
myCoreId='key in your device core id here'


token='access_token='+myAccessToken
urlToEvents=baseUrl+'events?'+token
# this api returns two lines:
# 1. line: event source
# 2. line: event contents json

# ------ Subroutine for calling cloud function
def gotoSafeMode():
   "This calls cloud function safe"
   urlToFunction=baseUrl+coreid+'/safe'
   try:
       r2 = requests.post(urlToFunction, data = {'arg':'value', 'access_token': myAccessToken}, timeout=8.141)
       r2Resp = r2.json()
       if r2Resp.has_key('return_value'):
           if r2Resp['return_value'] == 1:
               print "Safe mode successful"
   except requests.exceptions.Timeout:
       print "Cloud function call for safe mode timeout"
   return;

# --------- Main listener  --------
r = requests.get(urlToEvents, stream=True)
eventsource = "not set"
for line in r.iter_lines():
    if line:
#        print "Got:", line
        try:
            cleanedline = re.sub(r'^.*{',"{",line) #clean line until json array
            jsoned = json.loads(cleanedline);
            print jsoned['published_at'],eventsource,"\""+jsoned['data']+"\""
            if jsoned['data'] == "online" and jsoned['coreid'] == myCoreId:
                coreid = jsoned['coreid'];
                print "Trying to get to safe mode"
                gotoSafeMode()
        except ValueError:
            elements = line.split(":")
            if len(elements) > 1:
                #               print "First:",elements[0]," second:",elements[1]
                if elements[0] == "" and elements[1] == "ok":
                    print "Connection ok"
                if elements[0] == "event":
                    eventsource=elements[1]
                    if elements[1] == " spark/status/safe-mode":
                        print "Device is now in Safe mode"
