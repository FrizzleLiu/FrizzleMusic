package com.frizzle.frizzlemusic;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        Button button = findViewById(R.id.btn);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                requestPerms();
            }
        });
    }


    private void requestPerms() {
        //权限,简单处理下
        if (Build.VERSION.SDK_INT>Build.VERSION_CODES.N) {
            String[] perms= {Manifest.permission.WRITE_EXTERNAL_STORAGE};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms,200);
            }else {
                openAudio();
            }
        }
    }

    private void openAudio() {
        FrizzlePlayer frizzlePlayer = new FrizzlePlayer();
        String input = new File(Environment.getExternalStorageDirectory(),"input.mp3").getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(),"output.pcm").getAbsolutePath();
        frizzlePlayer.openAudio(input, output);
    }
}
