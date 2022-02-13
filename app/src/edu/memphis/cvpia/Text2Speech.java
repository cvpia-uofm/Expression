package edu.memphis.cvpia;

import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.util.Log;

public class Text2Speech{
	private TextToSpeech txt2speech;
	private final String[][] FEEDBACKLIST = {{"down,","up,"}, // 0: pitch
			{"right,","left,"}, // 1: yaw
			{"rolld,","unroll,"}, // 2: roll
			{"Brow-Up,","Brow-Down,"}, //3: Eye Brow
			{" "," "}, //4: Left Eye
			{" "," "}, //5: Right Eye
			{"Open,","Close,"}, //6: mouth
			{"Stretch,","Unstretch,"} //7: Lip Corner Distance	
			};
	
	public Text2Speech(Context cntx) {
		txt2speech = new android.speech.tts.TextToSpeech(cntx, null);
		txt2speech.setSpeechRate((float) 1.5);
	}
	
	public void close(){
		txt2speech.stop();
		txt2speech.shutdown();
	}
	
	public void speakOut(String text){
		txt2speech.speak(text, android.speech.tts.TextToSpeech.QUEUE_FLUSH, null);
		//close();
	}
	
	// To produce auditory feedback based on received features
	public void sonify(String feature){
		if(!feature.toLowerCase().equals("no face")){ // Do only if face is found
			Log.i("sonify", feature);
			//String[] events = feature.split(",");
			//String inputtoTTS = "";
			String inputtoTTS = feature;
			// Put the values of the features in this row matrix 
			/*for (int i=0;i<events.length;i++)
				if(Double.valueOf(events[i])>0.5){
					inputtoTTS = inputtoTTS + FEEDBACKLIST[i][0];
				}else if(Double.valueOf(events[i])<-0.5){
					inputtoTTS = inputtoTTS + FEEDBACKLIST[i][1];
				}*/
			if(!txt2speech.isSpeaking()){
				Log.i("sonify", inputtoTTS);
				speakOut(inputtoTTS);
			}
		}
	}
}
