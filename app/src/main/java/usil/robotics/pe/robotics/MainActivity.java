package usil.robotics.pe.robotics;

import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "Robotica";

    Button connectBt, startPositionBtn, sendAnglesBtn, startAnim;

    EditText angle1, angle2, angle3;

    BluetoothArduino bt;

    boolean connected = false;

    boolean animating = false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        connectBt = (Button) findViewById(R.id.connectBt);
        startPositionBtn = (Button) findViewById(R.id.startPosition);
        sendAnglesBtn = (Button) findViewById(R.id.sendAngles);
        startAnim = (Button) findViewById(R.id.startAnim);
        angle1 = (EditText) findViewById(R.id.angle1);
        angle2 = (EditText) findViewById(R.id.angle2);
        angle3 = (EditText) findViewById(R.id.angle3);

        bt = BluetoothArduino.getInstance("HC-05");

        connectBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(bt.isBluetoothEnabled()) {

                    if(!connected) {
                        connected = bt.Connect();
                        connectBt.setText("Apagar Bluetooth!");
                    } else {
                        connected = false;
                        bt.disableBluetooth();
                        connectBt.setText("Encender BT y conectar!");
                    }

                } else {
                    Toast.makeText(MainActivity.this, "Primero activa el bluetooth!", Toast.LENGTH_SHORT).show();
                }
            }
        });

        sendAnglesBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendAngles(angle1.getText().toString(), angle2.getText().toString(), angle3.getText().toString());
            }
        });

        startPositionBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendAngles("0", "90", "90");
            }
        });

        startAnim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if(!animating){
                    animHandler.post(r);
                    startAnim.setText("Pa rar!");
                    animating = true;
                } else {
                    animating = false;
                    startAnim.setText("Iniciar animación!");
                    animHandler.removeCallbacks(r);
                }
            }
        });

    }

    Handler animHandler = new Handler();
    Runnable r = new Runnable() {
        @Override
        public void run() {
            startAnimation();
        }
    };

    private void sendAngles(String a1, String a2, String a3) {

        if(!connected){
            Toast.makeText(this, "Primero conecta el robot por bluetooth!", Toast.LENGTH_SHORT).show();
            return;
        }

        int aa1 = Integer.parseInt(a1);
        int aa2 = Integer.parseInt(a2);
        int aa3 = Integer.parseInt(a3);

        if(aa1 > 90 || aa2 > 90 || aa3 > 90){
            Toast.makeText(this, "Los angulos solo van de 0 a 90 grados!", Toast.LENGTH_SHORT).show();
            return;
        }

        if(a1.isEmpty() || a2.isEmpty() || a3.isEmpty()){
            Toast.makeText(this, "Completa todos los campos, por favor", Toast.LENGTH_SHORT).show();
            return;
        }

        if("0".equals(a1)){
            a1 = "00";
        }

        if("0".equals(a2)){
            a2 = "00";
        }

        if("0".equals(a3)){
            a3 = "00";
        }

        a1 = a1+"n";
        a2 = a2+"n";
        a3 = a3+"n";

        bt.SendMessage(a1);
        bt.SendMessage(a2);
        bt.SendMessage(a3);

        log(a1 + " " + a2 + " " + a3);

    }

    private void startAnimation(){
        log("starting animation");
        String anim = "0n 90n 90n " +
                        "45n 90n 90n " +
                        "45n 20n 90n " +
                        "0n 0n 0n " +
                        "0n 0n 90n " +
                        "35n 0n 90n " +
                        "0n 0n 90n " +
                        "45n 0n 90n " +
                        "90n 0n 90n " +
                        "90n 45n 90n " +
                        "45n 30n 90n " +
                        "0n 25n 90n " +
                        "24n 12n 90n " +
                        "36n 5n 90n " +
                        "15n 1n 90n ";
        final String [] animation = anim.split(" ");

        new Thread(new Runnable() {
            @Override
            public void run() {

                for (int i = 0; i<animation.length ; i++) {
                    bt.SendMessage(animation[i]);
                    try {
                        Thread.sleep(700);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    @Override
                    public void run() {
                        animHandler.post(this);
                    }
                });

            }
        }).start();
    }

    private void log(String text){
        Log.d(TAG, text);
    }

}
