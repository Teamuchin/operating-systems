#!/bin/bash
#SBATCH -A is0001                # account / proje adi (Proje adı, projelerim komutu ile iş verilebileceğini gördüğünüz bir proje olmalıdır.)
#SBATCH -p cpu2dq                 # kuyruk (partition/queue) adi (kuyruk adı, bosmakinalar komutu ile müsait olduğunu gördüğünüz bir kuyruk olmalıdır.)
#SBATCH -n 1                    # toplam cekirdek / islemci sayisi
#SBATCH -N 1    		# # toplam makina sayisi

chmod +x compute.sh
./compute.sh
