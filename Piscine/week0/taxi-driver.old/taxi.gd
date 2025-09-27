extends Sprite2D

var speed = 200
var velocity = Vector2.ZERO

func _process(delta):
	velocity = Vector2.ZERO
	
	if Input.is_action_pressed("ui_right"):
		velocity.x += 1
		#texture = load("res://taxi_right.png")
		texture = load("res://Sayan.png")
	if Input.is_action_pressed("ui_left"):
		velocity.x -= 1
		texture = load("res://Sayan.png")
		#texture = load("res://taxi_left.png")
	if Input.is_action_pressed("ui_down"):
		velocity.y += 1
		texture = load("res://Sayan.png")
		#texture = load("res://taxi_down.png")
	if Input.is_action_pressed("ui_up"):
		velocity.y -= 1
		texture = load("res://Sayan.png")
		#texture = load("res://taxi_up.png")
	
	position += velocity.normalized() * speed * delta
