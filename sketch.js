let mSerial;
let connectButton;
let readyToReceive = true;
let lastRequestTime = 0;
let requestInterval = 100;

let scenes = [];
let button1Pressed = false;
let button2Pressed = false;

let media = [
  { img: "chap1S.jpg", video: "chap1V.mp4" },
  { img: "chap2S.jpg", video: "chap2V.mp4" }
];

// 断线警报逻辑
let lastDataReceived = 0;
let disconnected = false;
const disconnectTimeout = 5000; // 5秒无响应则断线警告

function preload() {
  for (let i = 0; i < media.length; i++) {
    let vid = createVideo(media[i].video);
    vid.hide();
    vid.elt.muted = true;
    scenes.push({
      img: loadImage(media[i].img),
      video: vid,
      playing: false
    });
  }
}

function setup() {
  createCanvas(windowWidth, windowHeight);
  background(0);

  mSerial = createSerial();
  connectButton = createButton("🔌 连接 Arduino");
  connectButton.position(20, 20);
  connectButton.mousePressed(connectToSerial);

  textAlign(CENTER, CENTER);
  textSize(32);
  lastDataReceived = millis();
}

function draw() {
  background(0);
  let sceneWidth = width / 2;

  for (let i = 0; i < scenes.length; i++) {
    let x = i * sceneWidth;
    image(scenes[i].playing ? scenes[i].video : scenes[i].img, x, 0, sceneWidth, height);
  }

  // 检查断线
  if (millis() - lastDataReceived > disconnectTimeout) {
    disconnected = true;
  } else {
    disconnected = false;
  }

  if (disconnected) {
    fill(255, 0, 0, 180);
    noStroke();
    rect(0, height / 2 - 50, width, 100);
    fill(0);
    text("RESET NEEDED", width / 2, height / 2);
  }

  if (mSerial.opened() && readyToReceive && millis() - lastRequestTime > requestInterval) {
    mSerial.clear();
    mSerial.write(0xAB);
    lastRequestTime = millis();
    readyToReceive = false;
  }

  if (mSerial.availableBytes() > 0) {
    receiveSerial();
  }
}

function receiveSerial() {
  let line = mSerial.readUntil("\n").trim();
  if (!line) return;

  try {
    let json = JSON.parse(line);
    let data = json.data;
    button1Pressed = data.button1;
    button2Pressed = data.button2;

    if (button1Pressed) playScene(0);
    if (button2Pressed) playScene(1);

    lastDataReceived = millis();
  } catch (e) {
    console.error("\u274c JSON 解析失败:", e, line);
  }

  readyToReceive = true;
}

function playScene(index) {
  let scene = scenes[index];
  if (!scene.playing) {
    scene.playing = true;
    scene.video.time(0);
    scene.video.play();

    scene.video.onended(() => {
      scene.playing = false;
      scene.video.hide();
    });
  }
}

function stopScene(index) {
  let scene = scenes[index];
  if (scene.playing) {
    scene.video.stop();
    scene.playing = false;
    scene.video.hide();
  }
}

function keyPressed() {
  if (key === '1') playScene(0);
  if (key === '2') playScene(1);
  if (key === 'q' || key === 'Q') stopScene(0);
  if (key === 'w' || key === 'W') stopScene(1);
  if (key === ' ') location.reload();
}

function connectToSerial() {
  if (!mSerial.opened()) {
    mSerial.open(9600);
    readyToReceive = true;
    connectButton.remove();
  }
}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}
