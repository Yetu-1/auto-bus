import { Button, Col, Form, FormGroup, Row, Stack } from "react-bootstrap";
import { Link, useNavigate } from "react-router-dom";
import { FormEvent, useRef, useState } from "react";


export function CustomForm() {
    const emailRef = useRef<HTMLInputElement>(null);
    const passwordRef = useRef<HTMLInputElement>(null);
    const navigate = useNavigate();

    function handleSubmit(e: FormEvent) {
        e.preventDefault();
        let user = {username: emailRef.current!.value , password: passwordRef.current!.value};
        console.log(user);

        fetch('http://localhost:5138/register', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
            },
            body: JSON.stringify(user)
          }).then(response => {
            if(response.status === 200){
                console.log(response);
                navigate("/");
            }else{
                console.log(response);
                alert("Email Already Exists!");
                navigate("/register");
            }}
          ) 
    }

    return(
        <Form onSubmit={handleSubmit}>
            <Stack gap={4}>
                <FormGroup controlId="title">
                    <Form.Label>Name</Form.Label>
                    <Form.Control ref={emailRef} required />
                </FormGroup>

                <FormGroup controlId="password">
                    <Form.Label>Password</Form.Label>
                    <Form.Control ref={passwordRef} required type="password" />
                </FormGroup>

                <Button type="submit" variant="primary">Register</Button>
                {/* 
                <Link to="/register">
                        Not Registed? Register
                </Link> */}
            </Stack>
        </Form>
    );
}