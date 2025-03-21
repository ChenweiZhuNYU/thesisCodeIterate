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

// 提示相关变量
let showPrompt = [false, false];
let promptStartTime = [0, 0]; // 动画起始时间
let promptDuration = 3000;    // 总显示时长
let promptY = [0, 0];         // 当前 y 位置
let promptState = ["idle", "idle"]; // idle | enter | hold | exit

// 播放开始时间
let playStartTime = [0, 0];

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
  let now = millis();

  for (let i = 0; i < scenes.length; i++) {
    let x = i * sceneWidth;
    image(scenes[i].playing ? scenes[i].video : scenes[i].img, x, 0, sceneWidth, height);

    // 更新提示状态
    updatePrompt(i, now, sceneWidth);
  }

  if (mSerial.opened() && readyToReceive && now - lastRequestTime > requestInterval) {
    mSerial.clear();
    mSerial.write(0xAB);
    lastRequestTime = now;
    readyToReceive = false;
  }

  if (mSerial.availableBytes() > 0) {
    receiveSerial();
  }
}

function updatePrompt(index, now, sceneWidth) {
  const animationTime = 300; // 滑入/滑出动画时间(ms)
  let elapsed = now - promptStartTime[index];
  let xCenter = index * sceneWidth + sceneWidth / 2;
  let targetY = height - 50;

  if (promptState[index] === "enter") {
    let t = constrain(elapsed / animationTime, 0, 1);
    promptY[index] = lerp(height + 50, targetY, t);
    if (t >= 1) {
      promptState[index] = "hold";
    }
  } else if (promptState[index] === "hold") {
    promptY[index] = targetY;
    if (elapsed > promptDuration - animationTime) {
      promptState[index] = "exit";
    }
  } else if (promptState[index] === "exit") {
    let t = constrain((elapsed - (promptDuration - animationTime)) / animationTime, 0, 1);
    promptY[index] = lerp(targetY, height + 50, t);
    if (t >= 1) {
      promptState[index] = "idle";
      showPrompt[index] = false;
    }
  }

  if (showPrompt[index]) {
    noStroke();
    fill(255, 0, 0, 180);
    rectMode(CENTER);
    rect(xCenter, promptY[index], 300, 80, 20);
    fill(255);
    text("please try later", xCenter, promptY[index]);
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
      let now = millis();
      if (scenes[0].playing) {
        if (now - playStartTime[0] > 2000) {
          triggerPrompt(0);
        }
      } else {
        playScene(0);
      }
    }

    if (button2Pressed) {
      let now = millis();
      if (scenes[1].playing) {
        if (now - playStartTime[1] > 2000) {
          triggerPrompt(1);
        }
      } else {
        playScene(1);
      }
    }
  } catch (e) {
    console.error("❌ JSON 解析失败:", e, line);
  }

  readyToReceive = true;
}

function triggerPrompt(index) {
  showPrompt[index] = true;
  promptStartTime[index] = millis();
  promptState[index] = "enter";
}

function playScene(index) {
  let scene = scenes[index];
  if (!scene.playing) {
    scene.playing = true;
    scene.video.time(0);
    scene.video.play();
    playStartTime[index] = millis();

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
