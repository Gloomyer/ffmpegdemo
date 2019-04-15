package com.gloomyer.player;

import android.Manifest;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.gloomyer.player.player.VideoPalyer;
import com.gloomyer.player.player.VideoView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    VideoView mVideoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mVideoView = findViewById(R.id.video_view);

        ActivityCompat.requestPermissions(this,
                new String[]{
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE},
                321);
    }

    public void play(View view) {
        File dir = Environment.getExternalStorageDirectory();
        dir = new File(dir, "Movies");
        String input = new File(dir, "wtx.mkv").getAbsolutePath();
        VideoPalyer.play(input, mVideoView.getHolder().getSurface());
    }
}
