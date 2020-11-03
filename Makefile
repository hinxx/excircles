all:
	$(MAKE) -f Makefile.env

	$(MAKE) -C lib
	$(MAKE) -C sample/01-ft6206
	$(MAKE) -C sample/02-ili9341
	$(MAKE) -C sample/03-tsc2046
	$(MAKE) -C sample/04-ili9325d
	$(MAKE) -C sample/05-itdb02-calibrate
	$(MAKE) -C sample/06-itdb02
	$(MAKE) -C sample/07-pitft-calibrate
	$(MAKE) -C sample/08-pitft
	$(MAKE) -C sample/09-ssd1351

clean:
	$(MAKE) -C sample/09-ssd1351 clean
	$(MAKE) -C sample/08-pitft clean
	$(MAKE) -C sample/07-pitft-calibrate clean
	$(MAKE) -C sample/06-itdb02 clean
	$(MAKE) -C sample/05-itdb02-calibrate clean
	$(MAKE) -C sample/04-ili9325d clean
	$(MAKE) -C sample/03-tsc2046 clean
	$(MAKE) -C sample/02-ili9341 clean
	$(MAKE) -C sample/01-ft6206 clean
	$(MAKE) -C lib clean
