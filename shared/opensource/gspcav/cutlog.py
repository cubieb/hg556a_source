#! /usr/bin/env python

import syslog

journaux = [
    "/var/log/messages"    
    ]



for journal in journaux:
    fichier = open(journal)
    lignes = fichier.readlines()
    fichier.close
    souslignes = lignes[-5:]
    fichier = open(journal, 'w')
    fichier.writelines(souslignes)
    fichier.close()

syslog.syslog("cutlog.py: réduction des journaux systèmes") 
