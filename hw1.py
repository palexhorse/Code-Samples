"""
Aaron T Johnson - atj0015@unt.edu
CSCE 4444 HW1 - Web Crawler/Scraper
1.27.18
Desc:
A simple Python script which accesses the CNN RSS feed, parses the html, and
pulls the five most recent titles from the sections "Tech", "Travel", and "Health".
"""
import requests
from bs4 import BeautifulSoup
#TECH
# set your url to the appropriate rss feed
url = "http://rss.cnn.com/rss/money_technology.rss"
#pull the html info
r = requests.get(url)
#parse the html
soup = BeautifulSoup(r.content, "html.parser")
#trim the extra code from the body code
titles = soup.find_all("item")
#print the first five titles listed
print "Tech:"
x=0
for title in titles:
	if x>4:
		break
	else:
		print "- " + title.title.text
		x+=1

#repeat for Travel and Health!
#TRAVEL
url = "http://rss.cnn.com/rss/cnn_travel.rss"
r = requests.get(url)
soup = BeautifulSoup(r.content, "html.parser")
titles = soup.find_all("item")
print "Travel:"
x=0
for title in titles:
	if x>4:
		break
	else:
		print "- " + title.title.text
		x+=1

#HEALTH
url = "http://rss.cnn.com/rss/cnn_health.rss"
r = requests.get(url)
soup = BeautifulSoup(r.content, "html.parser")
titles = soup.find_all("item")
print "Health:"
x=0
for title in titles:
	if x>4:
		break
	else:
		print "- " + title.title.text
		x+=1



