package com.example.myfirstapp

import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.TextView
import androidx.activity.ComponentActivity
import java.text.SimpleDateFormat
import java.util.Locale
import java.util.Date
import android.content.Context
import android.os.Environment
import java.io.File
import java.io.FileWriter
import java.io.IOException
import android.webkit.WebView
import android.webkit.WebViewClient
import android.webkit.HttpAuthHandler
import android.net.http.SslError
import com.google.gson.Gson
import android.webkit.SslErrorHandler
import android.webkit.JavascriptInterface

import android.app.AlertDialog
import android.content.DialogInterface
import android.widget.EditText
import androidx.compose.ui.text.font.Typeface


//@Serializable
//data class AccClass(
//    val loggedData:String,
//    val dataSize:Int
//)

class MainActivity : ComponentActivity(),SensorEventListener{
    private lateinit var sensorManager: SensorManager
    private lateinit var accelerometer: Sensor
    private lateinit var myWebView:WebView
    private var isCollectingData: Boolean = false
    private val tag = "MainActivity"
    private var accelData: ArrayList<String> = ArrayList()
    //private val dateFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSSS", Locale.getDefault())
    private val dateFormat = SimpleDateFormat("ss.SSSS", Locale.getDefault())
    private var distance=0.0
    //private var txt=""


    inner class AndroidInterface {
        @JavascriptInterface
        fun sendMessage(message: String):String {
            //val jsMessage: TextView = findViewById(R.id.messageShow)
            if (message == "start") {
                Log.d("receiveMessage","start");
                // 根据消息开始收集加速度信息
                startDataCollection()
                //jsMessage.text=message;
                return ""
            } else if (message == "stop") {
                Log.d("receiveMessage","stop");
                // 根据消息停止收集加速度信息
                stopDataCollection()
                //jsMessage.text=message;
                return distance.toString()
            }
            return " "
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        //Log.e("MainActivity","start create")
        super.onCreate(savedInstanceState)
        setContentView(R.layout.mainactivity)

        // 获取传感器管理器实例
        sensorManager = getSystemService(SENSOR_SERVICE) as SensorManager
        accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)

        // 设置按钮的点击监听器
        val startButton: Button = findViewById(R.id.startButton)
        val stopButton: Button = findViewById(R.id.stopButton)
        startButton.setOnClickListener { startDataCollection() }
        stopButton.setOnClickListener { stopDataCollection() }
        loadJS()

    }

    private fun startDataCollection() {
        if (!isCollectingData) {
            isCollectingData = true
            accelData.clear()
            sensorManager.registerListener(this,accelerometer,10000)
            Log.d(tag, "Data collection started")
        }
    }

    private fun stopDataCollection() {
        if (isCollectingData) {
            isCollectingData = false
            sensorManager.unregisterListener(this)
            //saveDataToFile() // 当停止收集数据时保存数据
            calDistance()
            writeData()
            //val distTextView: TextView = findViewById(R.id.distanceShow)
            //distTextView.text = "distance: $distance m"
            Log.d(tag, "Data collection stopped")
        }
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {
       // 当传感器精度改变时触发
    }

    override fun onSensorChanged(event: SensorEvent) {
        //if (event.sensor.type == Sensor.TYPE_ACCELEROMETER) {
            val xAccel = event.values[0]
            val yAccel = event.values[1]
            val zAccel = event.values[2]
            val time = dateFormat.format(Date()) // 获取当前时间用你选定的格式
            //accelData.add("Time: $time, X: $xAccel, Y: $yAccel, Z: $zAccel") // 添加数据记录
            accelData.add("$time, $xAccel, $yAccel, $zAccel")

//            runOnUiThread {
//                val xAccelTextView: TextView = findViewById(R.id.xAccelTextView)
//                val yAccelTextView: TextView = findViewById(R.id.yAccelTextView)
//                val zAccelTextView: TextView = findViewById(R.id.zAccelTextView)
//
//                // 实时显示加速度值
//                xAccelTextView.text = "X acceleration: $xAccel"
//                yAccelTextView.text = "Y acceleration: $yAccel"
//                zAccelTextView.text = "Z acceleration: $zAccel"
//            }
    }

    private fun saveDataToFile() {
        val filename = "accel_data.txt"
        val fileContents = accelData.joinToString("\n")
        openFileOutput(filename, Context.MODE_PRIVATE).use {
            it.write(fileContents.toByteArray())
        }
    }

    private fun writeData(){
        val isExternalStorageWritable = Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED
        if (isExternalStorageWritable) {
            val externalStorageDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
            val fileName = "accel_data2.txt" // 你想要保存的文件名
            val targetFile = File(externalStorageDir, fileName)
            try {
                val fileWriter = FileWriter(targetFile)
                val fileContents = accelData.joinToString("\n")
                fileWriter.write(fileContents) // fileContents 是你的数据
                fileWriter.flush()
                fileWriter.close()
                Log.d("err write data","write data success")
                //MediaScannerConnection.scanFile(context, arrayOf(targetFile.absolutePath), null, null)
            } catch (e: IOException) {
                e.printStackTrace()
                Log.e("err write data","write data fail")
                Log.e("err write data", "write data fail: " + e.message)
                if (targetFile.exists()) {
                    Log.d("FileExists", "File exists")}
            }
        } else {
            Log.e("err permission","unable to access external storage")
        }
    }

    private fun loadJS(){
        myWebView=findViewById(R.id.webview)
        myWebView.settings.javaScriptEnabled = true
        myWebView.settings.domStorageEnabled = true
        myWebView.webViewClient = object : WebViewClient() {
            override fun onReceivedSslError(view: WebView?, handler: SslErrorHandler?, error: SslError?) {
                handler?.proceed()
            }
            override fun onReceivedHttpAuthRequest(
                view: WebView?,
                handler: HttpAuthHandler?,
                host: String?,
                realm: String?
            ) {
                // 处理基本的HTTP身份验证请求
                val username = "admin"
                val password = "robotics"
                // 自动填充用户名和密码
                view?.evaluateJavascript(
                    "javascript:document.getElementById('username_field').value = '$username';" +
                            "javascript:document.getElementById('password_field').value = '$password';",
                    null
                )
                handler?.proceed(username, password)
            }
        }
        val alertDialog = AlertDialog.Builder(this@MainActivity)
        alertDialog.setTitle("please insert URL")
        val input = EditText(this@MainActivity)
        input.setText("https://192.168.43.224:80/docs/index.html") // 默认网页地址
        alertDialog.setView(input)
        alertDialog.setPositiveButton("Confirm") { dialog, which ->
            val url = input.text.toString()
            myWebView.loadUrl(url)
        }
        alertDialog.setNegativeButton("Cancel") { dialog, which ->
            dialog.cancel()
        }
        alertDialog.show()
        //myWebView.loadUrl("http://www.baidu.com")
        //myWebView.loadUrl("http://180.101.50.188/")
        //myWebView.loadUrl("https://192.168.43.224:80/docs/index.html")
        myWebView.addJavascriptInterface(AndroidInterface(), "androidInterface")
    }

    private fun calDistance(){
        val timeArray = DoubleArray(accelData.size)
        val accelArray = Array(accelData.size) { DoubleArray(3) }
        var tmp=0.0;
        for (i in accelData.indices) {
            val data = accelData[i].split(", ")
            if (i==0){tmp=data[0].toDouble()}
            timeArray[i] = data[0].toDouble()-tmp;
            if (timeArray[i]<0) {timeArray[i]=timeArray[i]+60}
            accelArray[i][0] = data[1].toDouble()
            accelArray[i][1] = data[2].toDouble()
            accelArray[i][2] = data[3].toDouble()-9.7
        }
        //txt = timeArray.joinToString("\n")
        distance=IMU_cal(timeArray,accelArray)
    }

    external fun IMU_cal(oneDArray: DoubleArray, twoDArray: Array<DoubleArray>): Double
    companion object {
        init {
            System.loadLibrary("DD")
        }
    }
}
