
				<div class="subcontainer" id="loksteuerung">

					<div class="locoselect" id="directadress">
						<div class="locoselect_left" style="width: 8rem;">
							<div class="radiodiv">
								<input id="radio01" type="radio" name="protocol" checked/>
			  					<label for="radio01"><span></span>MM</label>
			  				</div>
			  				<div class="radiodiv">
								<input id="radio02" type="radio" name="protocol"/>
			  					<label for="radio02"><span></span>DCC</label>
			  				</div>
			  			</div><!--
			  		 --><div class="locoselect_left" style="width: 9.3rem;">
			  				<div class="textdiv">
				  				<p>Adresse:</p>
								<input id="adress_input" class="textfield" pattern="[0-9]*" type="number" name="adress" value="1"><br>
							</div>
						</div><!--
					 --><div class="locoselect_left" style="width: 5.2rem;">
							<a href=""><div class="button" id="confirm_adress" >OK</div></a>
						</div><!--
					 --><div class="locoselect_left" style="width: 11.5rem;">
							<a href=""><div class="button toggle_lokliste">Lokliste</div></a>
					 	</div>
					</div>

					<div class="locoselect" id="lokliste" style="display: none;">
						<div class="locoselect_left">
							<a href=""><div id="locoselector">Lokauswahl</div></a>
						
							<div id="locoselector_list">
									
							</div>
						</div><!--
					 --><div class="locoselect_left">
							<a href=""><div class="button toggle_lokliste">Adresse</div></a>
					 	</div>
					</div>

					<?php if ($side == 'left') { ?>
					<div class="controlscontainer">
						<div class="slider">
							<input type="range" id="rangeslider01" data-orientation="vertical" min="0" max="1000" value="0" step="1">
							<a id="changedirection" href=""><div class="button" id="change_direction">
								<div id="left" style="width: 100%; display: none;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #2FA938; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #adadad; float: right;"></div>
								</div>
								<div id="right" style="width: 100%; display: inline-block;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #adadad; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #2fa938; float: right;"></div>
								</div>
							</div></a>
						</div><!--
					 --><div class="functions" style="margin: 0 .5rem;">
							<a href=""><div class="function button">F0</div></a>
							<a href=""><div class="function button">F1</div></a>
							<a href=""><div class="function button">F2</div></a>
							<a href=""><div class="function button">F3</div></a>
							<a href=""><div class="function button">F4</div></a>
							<a href=""><div class="function button">F5</div></a>
							<a href=""><div class="function button">F6</div></a>
							<a href=""><div class="function button">F7</div></a>
						</div><!--
					 --><div class="functions">
							<a href=""><div class="function button">F8</div></a>
							<a href=""><div class="function button">F9</div></a>
							<a href=""><div class="function button">F10</div></a>
							<a href=""><div class="function button">F11</div></a>
							<a href=""><div class="function button">F12</div></a>
							<a href=""><div class="function button">F13</div></a>
							<a href=""><div class="function button">F14</div></a>
							<a href=""><div class="function button">F15</div></a>
						</div>
					</div>
					<?php }else{ ?>
					<div class="controlscontainer">
						<div class="functions">
							<a href=""><div class="function button">F0</div></a>
							<a href=""><div class="function button">F1</div></a>
							<a href=""><div class="function button">F2</div></a>
							<a href=""><div class="function button">F3</div></a>
							<a href=""><div class="function button">F4</div></a>
							<a href=""><div class="function button">F5</div></a>
							<a href=""><div class="function button">F6</div></a>
							<a href=""><div class="function button">F7</div></a>
						</div><!--
					 --><div class="functions" style="margin: 0 .5rem;">
							<a href=""><div class="function button">F8</div></a>
							<a href=""><div class="function button">F9</div></a>
							<a href=""><div class="function button">F10</div></a>
							<a href=""><div class="function button">F11</div></a>
							<a href=""><div class="function button">F12</div></a>
							<a href=""><div class="function button">F13</div></a>
							<a href=""><div class="function button">F14</div></a>
							<a href=""><div class="function button">F15</div></a>
						</div><!--
					 --><div class="slider">
							<input type="range" id="rangeslider01" data-orientation="vertical" min="0" max="1000" value="0" step="1">
							<a id="changedirection" href=""><div class="button" id="change_direction">
								<div id="left" style="width: 100%; display: none;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #2FA938; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #adadad; float: right;"></div>
								</div>
								<div id="right" style="width: 100%; display: inline-block;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #adadad; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #2fa938; float: right;"></div>
								</div>
							</div></a>
						</div>
					</div>
					<?php } ?>
					<div class="stopgocontainer" style="margin-bottom: .5rem;">
						<center>
							<a href=""><div class="stopgobutton button" id="stop_button_1" style="color: red;">STOP</div></a>
							<a href=""><div class="stopgobutton button" id="go_button_1" style="color: green;">GO</div></a>
						</center>
					</div>
				</div>
				<script type="text/javascript" src="js/websocket.js?1"></script>
				<script type="text/javascript" src="js/loksteuerung.js?9"></script>

