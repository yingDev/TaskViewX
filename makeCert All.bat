makecert -r -pe -n "CN=YingDev.com CA" -ss CA -sr CurrentUser -a sha256 -cy authority -sky signature -sv YingDevCA.pvk YingDevCA.cer

makecert -pe -n "CN=YingDev.com SPC" -a sha256 -cy end -sky signature -ic YingDevCA.cer -iv YingDevCA.pvk  -sv YingDevSPC.pvk YingDevSPC.cer

pvk2pfx -pvk YingDevSPC.pvk -spc YingDevSPC.cer -pfx YingDevSPC.pfx