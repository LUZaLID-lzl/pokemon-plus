package com.pokemonplus.simulator;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.view.Gravity;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.security.MessageDigest;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipEntry;

/** Extract immutable native art once per resource revision, in app-private storage. */
public final class LauncherActivity extends Activity {
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        FrameLayout root = new FrameLayout(this);
        root.setBackgroundColor(0xff101c22);
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(72,72,Gravity.CENTER);
        root.addView(new ProgressBar(this),params);
        setContentView(root);
        new Thread(() -> {
            try {
                prepareResources();
                runOnUiThread(() -> {
                    if (!isFinishing() && !isDestroyed()) {
                        startActivity(new Intent(this,GameActivity.class).addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT));
                        finish();
                    }
                });
            } catch (Exception failure) {
                Log.e("PokemonPlus","Resource preparation failed",failure);
                runOnUiThread(() -> new AlertDialog.Builder(this).setMessage("资源加载失败，请重新安装应用")
                    .setPositiveButton("关闭",(dialog,which) -> finish()).show());
            }
        },"PokemonPlus-resources").start();
    }
    private static void remove(File f) throws IOException {
        File[] children=f.listFiles();
        if(children!=null)for(File child:children)remove(child);
        if(f.exists() && !f.delete())throw new IOException("Cannot remove "+f.getName());
    }
    private void prepareResources() throws Exception {
        String expected;
        try(InputStream in=getAssets().open("resources.sha256")) {
            expected=new String(in.readAllBytes(),StandardCharsets.US_ASCII).trim();
        }
        File destination=new File(getFilesDir(),"resources");
        File stamp=new File(destination,".revision");
        if(stamp.isFile() && new File(destination,"scenes/0.bin").isFile()
            && new String(Files.readAllBytes(stamp.toPath()),StandardCharsets.US_ASCII).equals(expected))return;
        MessageDigest sha=MessageDigest.getInstance("SHA-256");
        byte[] buffer=new byte[65536];
        try(InputStream in=getAssets().open("resources.zip")) {
            int count;while((count=in.read(buffer))!=-1)sha.update(buffer,0,count);
        }
        StringBuilder actual=new StringBuilder();
        for(byte b:sha.digest())actual.append(String.format("%02x",b & 255));
        if(!actual.toString().equals(expected))throw new IOException("Resource archive checksum mismatch");
        File staging=new File(getFilesDir(),"resources-next");
        remove(staging);
        if(!staging.mkdirs())throw new IOException("Cannot create resource directory");
        String prefix=staging.getCanonicalPath()+File.separator;
        try(ZipInputStream zip=new ZipInputStream(getAssets().open("resources.zip"))) {
            ZipEntry entry;
            while((entry=zip.getNextEntry())!=null) {
                File target=new File(staging,entry.getName());
                if(!target.getCanonicalPath().startsWith(prefix))throw new IOException("Invalid resource path");
                if(entry.isDirectory())continue;
                File parent=target.getParentFile();
                if(!parent.isDirectory() && !parent.mkdirs())throw new IOException("Cannot create resource folder");
                try(OutputStream out=new FileOutputStream(target)) {
                    int count;while((count=zip.read(buffer))!=-1)out.write(buffer,0,count);
                }
            }
        }
        Files.write(new File(staging,".revision").toPath(),expected.getBytes(StandardCharsets.US_ASCII));
        File previous=new File(getFilesDir(),"resources-previous");
        remove(previous);
        if(destination.exists() && !destination.renameTo(previous))throw new IOException("Cannot rotate resources");
        if(!staging.renameTo(destination)) {
            if(previous.exists())previous.renameTo(destination);
            throw new IOException("Cannot install resources");
        }
        remove(previous);
    }
}
