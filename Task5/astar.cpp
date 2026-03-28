#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
using std::placeholders::_1;

struct ANode {
  float f, g;
  int x, y;
  bool operator>(const ANode& o) const { return f > o.f; }
};

class AStarPlanner : public rclcpp::Node {
public:
  AStarPlanner() : Node("astar_planner") {
    auto qos = rclcpp::QoS(1).transient_local().reliable();
    sub_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
      "/map", qos, std::bind(&AStarPlanner::mapCallback, this, _1));
    pub_ = this->create_publisher<nav_msgs::msg::Path>("/path", qos);
    RCLCPP_INFO(this->get_logger(), "Waiting for map...");
  }
private:
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr pub_;

  void mapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
    int W = msg->info.width, H = msg->info.height;
    float res = msg->info.resolution;
    auto& data = msg->data;
    RCLCPP_INFO(this->get_logger(), "Map: %d x %d", W, H);
    
    auto idx=[&](int x,int y){ return y*W+x; };
    auto ok=[&](int x,int y){
      return x>=0&&x<W&&y>=0&&y<H&&data[idx(x,y)]>=0&&data[idx(x,y)]<50;
    };
    //int sx=1, sy=0, gx=W, gy=H;
    //auto find start
   int sx=0, sy=0;
    while(sx<W && sy<H && !ok(sx,sy)){
    	sx++;
    	sy++;
    }
    //autofind goal
    int gx=W-1, gy=H-1;
    while(gx>=0 && gy>=0 && !ok(gx,gy)){
    	gx--;
    	gy--;
    } 
    

/*
int sx=-1, sy=-1;
for(int y=0;y<H && sx==-1;y++){
for(int x=0;x<W && sx==-1;x++){
    if(ok(x, y)){
      sx=x; 
      sy=H-1;
    }
  }
  }
  
 int gx=-1, gy=-1;
for(int y=H-1 ;y>=0 && gx==-1;y++){
for(int x=W-1 ;x>=0 && gx==-1;x--){
    if(ok(x, y)){
      gx=x; 
      gy=y;
    }
  }
  }

//safety
if(sx ==-1 || gx == -1) {
	RCLCPP_ERROR(this->get_logger(), "No gap found ");
	return;
}

int sx = 1 , sy = 1;
int gx = W-2, gy = H-2;
*/
   RCLCPP_INFO(this->get_logger(),"Start: (%d,%d), Goal : (%d,%d)", sx,sy,gx,gy);
    
    std::vector<float> g(W*H, 1e9);
    std::vector<bool> closed(W*H, false);
    std::unordered_map<int,int> came;
    std::priority_queue<ANode,std::vector<ANode>,std::greater<ANode>> open;
    g[idx(sx,sy)]=0;
    ANode sn; sn.f=(float)std::hypot(gx-sx,gy-sy); sn.g=0; sn.x=sx; sn.y=sy;
    open.push(sn);
    int dx[]={1,-1,0,0,1,1,-1,-1};
    //int dx[]={1,-1,0,0};
    int dy[]={0,0,1,-1,1,-1,1,-1};
    //int dy[]={0,0,1,-1};
    float dc[]={1,1,1,1,1.414f,1.414f,1.414f,1.414f};
    //\float dc[]={1,1,1,1};
    bool found=false;
    while(!open.empty()){
      ANode cur=open.top(); open.pop();
      if(closed[idx(cur.x,cur.y)]) continue;
      closed[idx(cur.x,cur.y)]=true;
      if(cur.x==gx&&cur.y==gy){found=true;break;}
      for(int d=0;d<8;d++){
        int nx=cur.x+dx[d], ny=cur.y+dy[d];
        if(!ok(nx,ny)||closed[idx(nx,ny)]) continue;
        float ng=g[idx(cur.x,cur.y)]+dc[d];
        if(ng<g[idx(nx,ny)]){
          g[idx(nx,ny)]=ng;
          came[idx(nx,ny)]=idx(cur.x,cur.y);
          ANode nb; nb.g=ng; nb.f=ng+(float)std::hypot(gx-nx,gy-ny); nb.x=nx; nb.y=ny;
          open.push(nb);
        }
      }
    }
    if(!found){RCLCPP_ERROR(this->get_logger(),"No path!");return;}
    std::vector<std::pair<int,int>> path;
    int cur=idx(gx,gy);
    while(cur!=idx(sx,sy)){path.push_back({cur%W,cur/W});cur=came[cur];}
    path.push_back({sx,sy});
    std::reverse(path.begin(),path.end());
    nav_msgs::msg::Path pmsg;
    pmsg.header.frame_id="map";
    pmsg.header.stamp=this->now();
    for(auto& p:path){
      geometry_msgs::msg::PoseStamped ps;
      ps.header=pmsg.header;
      ps.pose.position.x=p.first*res+msg->info.origin.position.x;
      ps.pose.position.y=p.second*res+msg->info.origin.position.y;
      ps.pose.orientation.w=1.0;
      pmsg.poses.push_back(ps);
    }
    pub_->publish(pmsg);
    RCLCPP_INFO(this->get_logger(),"Path published! %zu poses",path.size());
  }
};

int main(int argc,char**argv){
  rclcpp::init(argc,argv);
  rclcpp::spin(std::make_shared<AStarPlanner>());
  rclcpp::shutdown();
  return 0;
}

