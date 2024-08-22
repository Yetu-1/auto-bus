import express from "express";
import bodyParser from "body-parser";
import pg from "pg"
import cors from "cors"
import axios from "axios"

const db = new pg.Client({
  user: "postgres",
  host: "localhost",
  database: "auto-bus",
  password: "swordfish",
  port: 5432,
});

let current_user_id = ""
const tarrif = 18.336;

const app = express();
const port = 5138;

db.connect();

app.use(bodyParser.json());
//app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static("public"));

app.use(cors({
    origin: "*",
    methods: ['GET', 'POST'],
    allowedHeaders: "*"
}));

app.get("/", async (req, res) => {
    try{
        const result = await db.query("SELECT * FROM users");
        //console.log(result.rows);
        const users = result.rows;
        res.json(users);
      }catch(err) {
        console.log(err);
        res.sendStatus(400);
    }
});

app.post("/esp-register", (req, res) => {
  console.log(req.body);
  current_user_id = req.body.id;
  if(current_user_id === null) {
    res.sendStatus(400);
  }
  res.sendStatus(200);
});

app.post("/esp-metrics", async (req, res) => {
  console.log(req.body);

  const uid = req.body.uid;
  let coor1 = {
    long: req.body.longitude,
    lat: req.body.latitude
  }
  const curr_time = req.body.curr_time;

  try{
    const checkResult = await db.query("SELECT * FROM users WHERE user_id = $1", [uid]);
    
    if(checkResult.rows.length == 0){ // uid does not exist
      console.log("USER NOT REGISTERED");
      res.sendStatus(404);
    }else {
      //console.log(coor1);
      try{
        const checkResult = await db.query("SELECT * FROM passengers WHERE uid = $1", [uid]);
        
        if(checkResult.rows.length > 0){
          // id already exists
          const result = await db.query("SELECT * FROM passengers WHERE uid = $1", [uid]); // get source longitude and latitude
          
          console.log(result.rows);

          let coor2 = {
            long: result.rows[0].longitude,
            lat:  result.rows[0].latitude
          }

          try{
            // Delete from db
            const resp = await db.query("DELETE FROM passengers WHERE uid = $1", [uid]);
          }catch {
            res.sendStatus(400);
          }
          
          // get the distance between the coordinates using the open street map route api
          let route_data = await getRouteData(coor1, coor2);
          console.log(route_data);
          const resp = await bill_user(uid, route_data.distance);

          res.sendStatus(200);
        }else{
          // id does not exist. Save passenger in array
          const result = await db.query("INSERT INTO passengers (uid, longitude, latitude, time) VALUES ($1, $2, $3, $4)",[uid, coor1.long, coor1.lat, curr_time]);
          console.log("Desitination Location saved in database")
          res.sendStatus(200);
        }
      }catch(err) {
        console.log(err);
        res.sendStatus(400);
      }
    }
  }catch(err) {
    console.log(err);
  }
});

app.post("/register", async (req, res) => {
  //console.log(req.body);
  const username = req.body.username;
  const password = req.body.password;
  const user_id = current_user_id;
  const balance = 10000;

  console.log(user_id);

  try{
    const checkResult = await db.query("SELECT * FROM users WHERE username = $1", [username]);
    
    if(checkResult.rows.length > 0){
      res.sendStatus(400) // Email already exists.
    }else{
      const result = await db.query("INSERT INTO users (username, password, user_id, balance) VALUES ($1, $2, $3, $4)",[username, password, user_id, balance]);
      console.log(result);
      res.sendStatus(200);
    }
  }catch(err) {
    console.log(err);
    res.sendStatus(400);
  }
});

app.post("/login", async (req, res) => {
  const email = req.body.username;
  const password = req.body.password;
  try{
    const result = await db.query("SELECT * FROM users WHERE email = $1", [email]);

    if(result.rows.length > 0){
      const user = result.rows[0];
      const storedPassword = user.password;

      if(password === storedPassword){
        // res.render("secrets.ejs")
      }else {
        res.send("Incorrect Password")
      }
    }else{
      res.send("User not found");
    }
  }catch(err) {
    console.log(err);
  }
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

async function getRouteData(coor1, coor2) {
  let data = {
    distance: 0,
    duration: 0
  }
  try {
    const result = await axios.get(`http://router.project-osrm.org/route/v1/driving/${coor1.long},${coor1.lat};${coor2.long},${coor2.lat}?overview=false`);
    data.distance = result.data.routes[0].distance;
    data.duration = result.data.routes[0].duration;
  } catch (error) {
    console.log(error);
  }
  return data;
}

function get_bill(distance) {
  return distance / tarrif;
}

async function bill_user(uid, distance) {
  try{
    // get user's current balance
    var result = await db.query("SELECT balance FROM users WHERE user_id = $1", [uid]);
    //console.log(result.rows);
    var curr_balance = result.rows[0].balance;
    const bill = get_bill(distance);
    curr_balance -= bill;
    
    // save updated user balance
    result = await db.query("UPDATE users SET balance = $1 WHERE user_id = $2", [curr_balance, uid]);
    console.log("User balance successfully updated")
  }catch(err) {
    console.log(err);
  }
}