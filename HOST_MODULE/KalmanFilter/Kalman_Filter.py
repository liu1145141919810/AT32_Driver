import os
import torch
import configparser

# Use absolute path based on this file's location
_CONFIG_DIR = os.path.dirname(os.path.abspath(__file__))
config = configparser.ConfigParser()
config.read(os.path.join(_CONFIG_DIR, 'param.ini'))
class KalmanFilter:
    def __init__(self,F,H,B,Q,R):
        self.F=F;self.H=H;self.B=B;self.Q=Q;self.R=R
        self.x=None;self.P=None
    def init(self,x0,P0):
        self.x=x0;self.P=P0
    def predict(self,u):
        self.x=self.F@self.x+self.B@u
        self.P=self.F@self.P@self.F.T+self.Q
    def update(self,z): 
        S=self.H@self.P@self.H.T+self.R
        K=torch.linalg.solve(
           S.T,(self.P@self.H.T).T).T
        self.x=self.x+K@(z-self.H@self.x)
        I=torch.eye(self.P.shape[0],device=self.P.device)
        self.P=(I-K@self.H)@self.P@(I-K@self.H).T+K@self.R@K.T
        return self.x,self.P
    def run_sequence(self, x0, P0, z_seq, u_seq=None):
        T=z_seq.shape[0]
        x_est=torch.zeros((T,x0.shape[0]),device=x0.device)
        if u_seq is None:
            u_seq=torch.zeros((T,self.B.shape[1]),device=x0.device)
        self.init(x0,P0)
        for t in range(T):
            self.predict(u_seq[t])
            self.update(z_seq[t])
            x_est[t]=self.x.reshape(-1)
        return x_est
class Runner:
    def __init__(self,F=None,H=None,B=None,Q=None,R=None):
        self.device=torch.device("cuda" if torch.cuda.is_available() else "cpu")
        if F is None:
            F=torch.tensor(eval(config.get('F','F'))).to(self.device)
        if H is None:
            H=torch.tensor(eval(config.get('H','H'))).to(self.device)
        if B is None:
            B=torch.tensor(eval(config.get('B','B'))).to(self.device)
        if Q is None:
            Q=torch.tensor(eval(config.get('Q','Q'))).to(self.device)
        if R is None:
            R=torch.tensor(eval(config.get('R','R'))).to(self.device)
        self.F=torch.tensor(F,dtype=torch.float32)
        self.H=torch.tensor(H,dtype=torch.float32)
        self.B=torch.tensor(B,dtype=torch.float32)
        self.Q=torch.tensor(Q,dtype=torch.float32)
        self.R=torch.tensor(R,dtype=torch.float32)

    def run(self,x0,z_seq,P0=None,u_seq=None):
        x0=torch.tensor(x0,device=self.device,dtype=torch.float32)
        z_seq=torch.tensor(z_seq,device=self.device,dtype=torch.float32)
        x0=x0.reshape(1,-1)
        z_seq=z_seq.reshape(z_seq.shape[0],-1)

        kf=KalmanFilter(self.F,self.H,self.B,self.Q,self.R)
        if P0 is None:
            P0=torch.eye(self.F.shape[0],device=self.F.device)
        P0=torch.tensor(P0,device=self.F.device,dtype=torch.float32)
        if u_seq is None:
            u_seq=torch.zeros((z_seq.shape[0],self.B.shape[1]),device=self.F.device)
        u_seq=torch.tensor(u_seq,device=self.F.device,dtype=torch.float32)
        x_est=kf.run_sequence(x0,P0,z_seq,u_seq)
        if self.device.type=="cuda":
            x_est=x_est.cpu()
        return x_est
if __name__=="__main__":
    runner = Runner()