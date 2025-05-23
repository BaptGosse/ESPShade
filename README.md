# <div align="center">Hi there 👋, bienvenue sur le projet **ESPShade**

# 💫 En quoi consiste-t-il ?

Ce projet a pour but de transformer, de façon DIY, des volets roulants manuels en automatiques, avec une application web et mobile

# 👨‍💻 Quels outils/matériels utiliser ?

J'utilise personnellement un ESP32 programmé en C/C++, via l'IDE "Arduino IDE", avec un moteur de 60 RPM à haut couple, pour ne pas que cela galère lors de l'ouverture, et un boitier modélisé et imprimé en 3D

# 📝 Il y aura-t-il une documentation ?

Oui, il y aura une documentation faite sur ce même repo GitHub. De plus, tout ce qui est fait ici va évouluer, être mis à jour, et surtoût, tout est OpenSource, et vous pouvez le réutiliser comme vous le souhaitez, faire les modifications que vous souhaitez. Alors foncez :)

# Prérequis :

Il vous faudra obligatoirement installer ces bibliothèques avant utilisation, car celles-ci ne sont pas incluses de base

- **ESPAsyncWebServer** de ESP32Async (téléchargeable dans l'IDE d'Arduino) : https://github.com/ESP32Async/ESPAsyncWebServer
- **AsyncTCP** de ESP32Async (téléchargeable dans l'IDE d'Arduino) : https://github.com/ESP32Async/AsyncTCP
- **ArduinoJson** de Benoit Blanchon (téléchargeable dans l'IDE d'Arduino) : https://github.com/bblanchon/ArduinoJson
- **WiFiManager** de tzapu (téléchargeable dans l'IDE d'Arduino) : https://github.com/tzapu/WiFiManager

Ainsi que toutes les bibliothèques installées de base quand vous paramétrez l'IDE d'Arduino pour les ESP32. Veillez à bien respecter toutes les licences associées à toutes ces différentes bibliothèques. Un grand merci à toutes les personnes qui développent et maintiennent ces bibliothèques qui me simplifient énormément la vie.