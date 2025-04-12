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
}

function draw() {
  background(0);
  let sceneWidth = width / 2;

  for (let i = 0; i < scenes.length; i++) {
    let x = i * sceneWidth;
    image(scenes[i].playing ? scenes[i].video : scenes[i].img, x, 0, sceneWidth, height);
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

    if (button1Pressed) {
      playScene(0);
    }

    if (button2Pressed) {
      playScene(1);
    }
  } catch (e) {
    console.error("❌ JSON 解析失败:", e, line);
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
