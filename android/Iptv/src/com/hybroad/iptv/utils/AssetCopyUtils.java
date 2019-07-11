package com.hybroad.iptv.utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import com.hybroad.iptv.log.SLog;

import android.content.res.AssetManager;

public class AssetCopyUtils {
	public static void CopyAssets(AssetManager am, String assetDir, String dir) throws Exception{

        SLog.pink("Copying Assets: " + assetDir+ " | " + dir);
		String[] files = null;
		File mWorkingPath = new File(dir);

		try {
			files = am.list(assetDir);
		} catch (Exception e) {
			throw new RuntimeException(e);
		}

		if (!mWorkingPath.exists()) {

			if (!mWorkingPath.mkdirs()) {

			}
		}

		for (int i = 0; files!=null&&i<files.length; i++) {
			try {
				String fileName = files[i];
				if (!fileName.contains(".")) {
					if (0 == assetDir.length()) {
						CopyAssets(am,fileName, dir +"/"+fileName + "/");
					} else {
						CopyAssets(am,assetDir + "/" + fileName, dir +"/"+ fileName + "/");
					}
					continue;
				}
                // SLog.pink("iptv_logging", "Copying Assets: " + mWorkingPath + "/" + fileName);
				File outFile = new File(mWorkingPath, fileName);
				if (outFile.exists())
					outFile.delete();
				InputStream in = null;
				if (0 != assetDir.length())
					in = am.open(assetDir + "/" + fileName);
				else
					in = am.open(fileName);
				OutputStream out = new FileOutputStream(outFile);

				byte[] buf = new byte[1024];
				int len = -1;
				while ((len = in.read(buf)) > 0) {
					out.write(buf, 0, len);
				}
				in.close();
				out.close();
			} catch (Exception e) {
				throw new RuntimeException(e);
			}
		}
	}
}
