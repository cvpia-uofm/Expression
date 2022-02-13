package edu.memphis.cvpia;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfByte;
import org.opencv.core.MatOfInt;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;
import org.opencv.objdetect.Objdetect;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.WindowManager;

public class EGlassActivity extends Activity implements CvCameraViewListener2 {

	private CameraBridgeViewBase mOpenCvCameraView;
	private boolean allowedToSendData = true;
	private long lastFrameMillisecAgo = 0;
	private long startTime = 0, endTime = 0;
	private int countNoResponseFromServer = 0;
	private int TIMEOUTperFrame = 100; // milliseconds
	private int maxConsecutiveNoResponse = 50; // count
	private String serverIpAddress = "192.168.0.101";
//	private String serverIpAddress = "141.225.167.197";
	private Mat mRgba, mGray;

	private MenuItem mItemSwitchCamera = null;
	private boolean frontCamera = false;
	private Text2Speech txt2sp;

	private Socket socket;
	// OutputStream socketOutputStream;
	private BufferedOutputStream output;
	// BufferedReader input;
	private BufferedInputStream input;

	private static int BUFFSIZE = 928000;
	private String prevBE = "";
	private int waitLimit = 0;

	protected String sensorValue = "";
	protected float sensorValueFloat = 0;
	float[] inR = new float[16];
	float[] outR = new float[16];
	float[] I = new float[16];
	float[] gravity = new float[3];
	float[] geomag = new float[3];
	float[] orientVals = new float[3];

	final float pi = (float) Math.PI;
	final float rad2deg = 180 / pi;
	AsyncTask startAsynTask;
	int counter = 0;
	private CascadeClassifier mJavaDetector;
	private File mCascadeFile;
	private static final Scalar FACE_RECT_COLOR = new Scalar(0, 255, 0, 255);
	private float mRelativeFaceSize = 0.2f;
	private int mAbsoluteFaceSize = 0;
	boolean isFoundFace = false;

	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				try {
					// load cascade file from application resources
					InputStream is = getResources().openRawResource(
							R.raw.lbpcascade_frontalface);
					File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
					mCascadeFile = new File(cascadeDir,
							"lbpcascade_frontalface.xml");
					FileOutputStream os = new FileOutputStream(mCascadeFile);

					byte[] buffer = new byte[4096];
					int bytesRead;
					while ((bytesRead = is.read(buffer)) != -1) {
						os.write(buffer, 0, bytesRead);
					}
					is.close();
					os.close();

					mJavaDetector = new CascadeClassifier(
							mCascadeFile.getAbsolutePath());
					if (mJavaDetector.empty()) {
						Log.i("rev", "Failed to load cascade classifier");
						mJavaDetector = null;
					} else
						Log.i("rev", "Loaded cascade classifier from "
								+ mCascadeFile.getAbsolutePath());

					cascadeDir.delete();

				} catch (IOException e) {
					e.printStackTrace();
					Log.e("rev", "Failed to load cascade. Exception thrown: "
							+ e);
				}

				mOpenCvCameraView.enableView();
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		txt2sp = new Text2Speech(getBaseContext());
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		// /android.os.Debug.waitForDebugger();

		setContentView(R.layout.activity_fepsmain);

		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.EGlassOpenCvView_Front);

		mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
		mOpenCvCameraView.setCvCameraViewListener(this);

	}

	@Override
	public void onResume() {
		super.onResume();
		startTime = System.currentTimeMillis();
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_6, this,
				mLoaderCallback);
		System.out.println("onresume fired");

		// startAsynTask=new AccessNetwork(getBaseContext()).execute();

		new Thread() {
			public void run() {
				try {
					System.out.println("called");
					ConnectivityManager conMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
					boolean isConn = conMgr.getNetworkInfo(
							ConnectivityManager.TYPE_WIFI)
							.isConnectedOrConnecting();
					if (isConn) {
						socket = new Socket(serverIpAddress, 8000);
						if (socket != null) {
							output = new BufferedOutputStream(
									socket.getOutputStream(), BUFFSIZE);
							input = new BufferedInputStream(
									socket.getInputStream(), BUFFSIZE);
						}
						System.out.println("status is ok");
					} else {
						System.out.println("Can not connect");
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}.start();
	}

	@Override
	public void onPause() {
		super.onPause();

		System.out.println("onpaused fired");
		finish();

		// to destroy the activity when it goes sleep or glass is taken off from
		// head.

	}

	// Binary data must be base64 encoded
	private void sendOutput(byte[] buff) {
		try {

			// insert sensorvalue at the begining

			byte[] newBuff = new byte[buff.length + 5 + 1];
			System.arraycopy(buff, 0, newBuff, 0 + 5, buff.length);
			newBuff[buff.length + 0 + 5] = ((byte) 28);

			// //////////////Sensor Input
			byte[] sensorBytes = new byte[4];
			ByteArrayOutputStream bas = new ByteArrayOutputStream();
			DataOutputStream ds = new DataOutputStream(bas);
			try {
				// ds.writeFloat((float) converttoAngle(sensorValueFloat));
				ds.writeFloat(0);
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			sensorBytes = bas.toByteArray();
			for (int i = 0; i < 4; i++)
				newBuff[i] = (byte) sensorBytes[i];
			newBuff[4] = ((byte) 28);

			// //////////////Sensor Input

			output.write(newBuff, 0, newBuff.length);
			output.flush();

			// output.write(buff, 0, buff.length);
			// output.flush();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	float converttoAngle(float sensorValue) {
		float angle = 0;
		if (sensorValue <= 0)
			angle = (-1) * (float) (90.00 + sensorValue);
		// angle = (sensorValue > -90.00)? (float) (90.00 + sensorValue) :
		// (float)90.00 - Math.abs(sensorValue) ;
		else
			// angle = (float)270 + sensorValue;
			angle = (sensorValue > 90) ? (-1) * ((float) 270 + sensorValue)
					+ 180 : (float) 270 - sensorValue;

		return angle;
	}

	private void sendOutput(String output) {
		sendOutput(output.getBytes());
	}

	private String readInput(int timeOutMilliSec) {
		String retVal = "";
		byte[] bytes = new byte[255];
		long startcount = System.currentTimeMillis();
		while (true) {
			try {
				if (input.available() > 0) {
					int bytesRead = input.read(bytes);
					retVal = new String(bytes, 0, bytesRead);
					break;
				} else if ((System.currentTimeMillis() - startcount) > timeOutMilliSec) {
					break;
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return retVal;
	}

	private void transmitFrameIfAllowed(Mat mGray) {
		// /android.os.Debug.waitForDebugger();
		if (allowedToSendData || counter % 2 == 0) {
			MatOfByte buff = new MatOfByte();
			Highgui.imencode(".jpg", mGray, buff, new MatOfInt(
					Highgui.IMWRITE_JPEG_QUALITY, 80));
			sendOutput(Base64.encode(buff.toArray(), Base64.NO_WRAP));
			Log.w("transmitFrameIfAllowed_160", "Frame Transmitted");
			//sallowedToSendData = false;
			lastFrameMillisecAgo = System.currentTimeMillis();
			System.out.println("sent: " + counter);
		}
		allowedToSendData = true;
		// Checks for server response in shorts bursts over
		// several frames because we don't need to send
		// all the frames but we can't hold the image display
		// for long because that'll hamper android process
		String checkResponse = readInput(100);// initial valu was 15
		Log.w("transmitFrameIfAllowed_169", "Checking Response ...");
		if (checkResponse.contains("Got Data:")) {
			String toSonify = checkResponse.substring(
					checkResponse.lastIndexOf(":") + 1, checkResponse.length());
			if (toSonify.equalsIgnoreCase("No Face Found"))
				isFoundFace = false;
			// if (!toSonify.isEmpty()) {
			if (toSonify.equalsIgnoreCase(prevBE)) {
				/*
				 * if (waitLimit == 0) { txt2sp.sonify(toSonify); waitLimit = 5;
				 * } else waitLimit--;
				 */

			} else {
				txt2sp.sonify(toSonify);
				prevBE = toSonify;
				waitLimit = 5;
			}

			// }else prevBE = toSonify;
			// allowedToSendData = true;
			countNoResponseFromServer = 0;
			Log.w("transmitFrameIfAllowed_173", "Check response: "
					+ checkResponse);
		} else if (((System.currentTimeMillis() - lastFrameMillisecAgo) > TIMEOUTperFrame)) {// sending
																								// 10
																								// fame
																								// per
																								// sec
			allowedToSendData = true;
			countNoResponseFromServer++;
			Log.w("transmitFrameIfAllowed_178",
					"TIMEOUT. No response"
							+ " from server. Sending next frame "
							+ String.valueOf(countNoResponseFromServer));
		}
		if (countNoResponseFromServer >= maxConsecutiveNoResponse) {
			Log.e("transmitFrameIfAllowed_183", "Server Non-Responsive");
			// finish();
		}
	}

	@Override
	public void onDestroy() {
		System.out.println("ondestroy fired");
		/*
		 * if(!startAsynTask.isCancelled()) { startAsynTask.cancel(true); }
		 */
		super.onDestroy();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
		mOpenCvCameraView = null;
		txt2sp.close();
		sendOutput(Character.toString((char) 4));
		android.os.Process.killProcess(android.os.Process.myPid());

		try {
			mRgba.release();
			mGray.release();
			mRgba = null;
			mGray = null;
			input.close();
			output.close();
			socket.shutdownInput();
			socket.shutdownOutput();
			socket.close();
			socket = null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.exit(0);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		mItemSwitchCamera = menu.add("Switch Camera");
		return true;
	}

	public boolean onOptionsItemSelected(MenuItem item) {
		String toastMesage = new String();
		if (item == mItemSwitchCamera) {
			mOpenCvCameraView.setVisibility(SurfaceView.GONE);
			mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.EGlassOpenCvView_Front);
			mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
			mOpenCvCameraView.setCvCameraViewListener(this);
			mOpenCvCameraView.enableView();

		}
		return true;
	}

	@Override
	public void onCameraViewStarted(int width, int height) {
		mGray = new Mat();
		mRgba = new Mat();
	}

	@Override
	public void onCameraViewStopped() {
		mGray.release();
		mRgba.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		counter++;
		System.out.println("counter :" + counter);
		mRgba = inputFrame.rgba();
		mGray = inputFrame.gray();
		// Core.flip(mGray, mGray, 1);
		Mat cropped = null;
		if (isFoundFace == false) {
			if (mAbsoluteFaceSize == 0) {
				int height = mGray.rows();
				if (Math.round(height * mRelativeFaceSize) > 0) {
					mAbsoluteFaceSize = Math.round(height * mRelativeFaceSize);
				}
			}

			MatOfRect faces = new MatOfRect();
			if (mJavaDetector != null)
				mJavaDetector.detectMultiScale(mGray, faces, 1.1, 3,
						Objdetect.CASCADE_SCALE_IMAGE, new Size(20, 20),
						new Size());
			Rect[] facesArray = faces.toArray();
			for (int i = 0; i < facesArray.length; i++) {
				isFoundFace = true;
				float minArea = 1600;
				Log.i("face size", "" + facesArray[i].width
						* facesArray[i].height);
				if ((facesArray[i].width * facesArray[i].height) < 1600) {
					txt2sp.sonify("come closer");
				}
				// Core.rectangle(mRgba, facesArray[i].tl(), facesArray[i].br(),
				// FACE_RECT_COLOR, 3);
				int x1, y1, x2, y2, x3, y3, x4, y4;
				int width = facesArray[i].width;
				int height = facesArray[i].height;
				x1 = facesArray[i].x - width / 2;
				y1 = facesArray[i].y - height / 2;
				x2 = x1 + 3 * width / 2;
				y2 = facesArray[i].y - height / 2;
				x3 = facesArray[i].x - width / 2;
				y3 = facesArray[i].y + 3 * height / 2;
				x4 = facesArray[i].x + 3 * width / 2;
				y4 = facesArray[i].y + 3 * height / 2;
				int imgWidth = mGray.width();
				int imgheight = mGray.height();
				Log.i("image size", "" + imgWidth * imgheight);
				if (x1 <= 0 && y1 <= 0) {
					x1 = 0;
					y1 = 0;
					x2 = 2 * width;
					y2 = 0;
					x3 = 0;
					y3 = 2 * height;
					x4 = 2 * width;
					y4 = 2 * height;
					txt2sp.sonify("Face in top left");
				} else if (x2 >= imgWidth && y2 <= 0) {
					x1 = imgWidth - 2 * width;
					y1 = 0;
					x2 = imgWidth;
					y2 = 0;
					x3 = imgWidth - 2 * width;
					y3 = 2 * height;
					x4 = imgWidth - 2 * width;
					y4 = 2 * height;

					txt2sp.sonify("Face in top right");

				} else if (x3 <= 0 && y3 >= imgheight) {
					x1 = 0;
					y1 = imgheight - 2 * height;
					x2 = 2 * width;
					y2 = imgheight - 2 * height;
					x3 = 0;
					y3 = imgheight;
					x4 = 2 * width;
					y4 = imgheight;

					txt2sp.sonify("Face in bottom left");

				} else if (x4 >= imgWidth && y4 >= imgheight) {
					x1 = imgWidth - 2 * width;
					y1 = imgheight - 2 * height;
					x2 = imgWidth;
					y2 = imgheight - 2 * height;
					x3 = imgWidth - 2 * width;
					y3 = imgheight;
					x4 = imgWidth;
					y4 = imgheight;

					txt2sp.sonify("Face in bottom right");

				} else if (x1 <= 0) {
					x1 = 0;
					x3 = 0;
					txt2sp.sonify("Face in left edge");

				} else if (y1 <= 0) {
					y1 = 0;
					y2 = 0;
					txt2sp.sonify("Face in top edge");

				} else if (x2 >= imgWidth) {
					x2 = imgWidth;
					x4 = imgWidth;
					txt2sp.sonify("Face in right edge");

				} else if (y4 >= imgheight) {
					y3 = imgheight;
					y4 = imgWidth;
					txt2sp.sonify("Face in bottom edge");

				} else {
					txt2sp.sonify("Face in center.");
				}
				Point tolLeft = new Point(x1, y1);
				Point bottomright = new Point(x4, y4);
				Rect roi = new Rect(tolLeft, bottomright);
				Core.rectangle(mRgba, tolLeft, bottomright, FACE_RECT_COLOR, 3);
				// cropped=new Mat(mGray, roi);

			}
		}

		if (isFoundFace == true) {
			Imgproc.resize(mGray, mGray, new Size(400, 400 * mGray.height()
					/ mGray.width()));
			// Imgproc.resize(mGray, mGray,new Size(600, 600));
			transmitFrameIfAllowed(mGray);
		}

		/*
		 * if(cropped!=null){ transmitFrameIfAllowed(cropped); //
		 * cropped.release(); cropped=null; }else{ Imgproc.resize(mGray,
		 * mGray,new Size(400, 400 * mGray.height() / mGray.width()));
		 * transmitFrameIfAllowed(mGray); }
		 */

		return mRgba;

	}

	public class AccessNetwork extends AsyncTask {

		private Context context;

		public AccessNetwork(Context context) {
			this.context = context;
		}

		@Override
		protected String doInBackground(Object... arg0) {
			try {
				ConnectivityManager conMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
				boolean isConn = conMgr.getNetworkInfo(
						ConnectivityManager.TYPE_WIFI)
						.isConnectedOrConnecting();
				if (isConn) {
					socket = new Socket(serverIpAddress, 9000);
					if (socket != null) {
						output = new BufferedOutputStream(
								socket.getOutputStream(), BUFFSIZE);
						input = new BufferedInputStream(
								socket.getInputStream(), BUFFSIZE);
					}
					System.out.println("status is ok");
				} else {
					System.out.println("Can not connect");
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			return "";
		}

	}

}
