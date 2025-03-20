let scenes = [];
let mSerial;
let readyToReceive = false;
let connectButton; // 连接 Arduino 的按钮

// 传感器数据
let distance1 = 0;
let distance2 = 0;
let showSensorData = false; // **默认隐藏传感器数据**

// 左右两侧的媒体文件
let media = [
  { img: "chap1S.jpg", video: "chap1V.mp4" }, // 左侧
  { img: "chap2S.jpg", video: "chap2V.mp4" }  // 右侧
];

function preload() {
  for (let i = 0; i < media.length; i++) {
    let vid = createVideo(media[i].video);
    vid.hide(); // 隐藏 HTML 视频
    vid.elt.muted = true; // 确保没有声音
    scenes.push({
      img: loadImage(media[i].img),
      video: vid,
      playing: false
    });
  }
}

function setup() {
  createCanvas(windowWidth, windowHeight);
  mSerial = createSerial();

  // 创建 Arduino 连接按钮
  connectButton = createButton("🔌 连接 Arduino");
  connectButton.position(20, 20);
  connectButton.mousePressed(connectToSerial);

  console.log("等待串口连接...");
}

function draw() {
  background(0);

  let sceneWidth = windowWidth / 2;
  let sceneHeight = windowHeight;

  // **遍历左右两个画面**
  for (let i = 0; i < scenes.length; i++) {
    let x = i * sceneWidth;
    
    if (scenes[i].playing) {
      image(scenes[i].video, x, 0, sceneWidth, sceneHeight);
    } else {
      image(scenes[i].img, x, 0, sceneWidth, sceneHeight);
    }
  }

  // **按 N 显示/隐藏传感器数据**
  if (showSensorData) {
    displaySensorData();
  }

  // **监听 Arduino 串口数据**
  if (mSerial.opened() && readyToReceive) {
    readyToReceive = false;
    mSerial.clear();
    mSerial.write(0xab); // 发送请求数据
  }

  if (mSerial.availableBytes() > 0) {
    receiveSerial();
  }
}

// **显示传感器数据**
function displaySensorData() {
  fill(255);
  textSize(32);
  textAlign(CENTER, TOP);

  // **左侧传感器数据**
  text(`传感器 1: ${distance1.toFixed(1)} cm`, width / 4, 20);

  // **右侧传感器数据**
  text(`传感器 2: ${distance2.toFixed(1)} cm`, (3 * width) / 4, 20);
}

// **播放视频**
function playScene(index) {
  let scene = scenes[index];

  if (!scene.playing) {
    scene.playing = true;
    scene.video.time(0); // 从头播放
    scene.video.play();

    // **视频播放完毕后，回到静态图片**
    scene.video.onended(() => {
      scene.playing = false;
      scene.video.hide();
    });
  }
}

// **解析串口数据**
function receiveSerial() {
  let line = mSerial.readUntil("\n");
  trim(line);
  if (!line) return;

  console.log("接收到数据: ", line);

  if (line.charAt(0) != "{") {
    console.log("数据格式错误: ", line);
    readyToReceive = true;
    return;
  }

  let json;
  try {
    json = JSON.parse(line);
  } catch (e) {
    console.error("JSON 解析失败: ", e);
    readyToReceive = true;
    return;
  }

  let data = json.data;
  console.log("解析后的数据: ", data);

  // **更新传感器数据**
  distance1 = data.distance1;
  distance2 = data.distance2;

  // **检测左侧传感器**
  if (distance1 < 5) {
    console.log("左侧触发视频");
    playScene(0); // 播放左侧视频
  }

  // **检测右侧传感器**
  if (distance2 < 5) {
    console.log("右侧触发视频");
    playScene(1); // 播放右侧视频
  }

  readyToReceive = true;
}

// **连接 Arduino**
function connectToSerial() {
  if (!mSerial.opened()) {
    mSerial.open(9600);
    readyToReceive = true;
    console.log("串口已连接");

    // **移除按钮**
    connectButton.remove();
  }
}

// **按 N 键显示/隐藏传感器数据**
function keyPressed() {
  if (key === 'n' || key === 'N') {
    showSensorData = !showSensorData;
    console.log(`📡 传感器数据显示: ${showSensorData ? "显示" : "隐藏"}`);
  }
}

// **窗口大小调整**
function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}
